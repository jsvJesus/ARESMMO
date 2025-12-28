#include "ARESMMOCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "UI/Game/GameHUDWidget.h"
#include "UI/Game/Inventory/InventoryWidget.h"
#include "UI/Game/Inventory/InventoryLayoutWidget.h"
#include "UI/Game/Inventory/InventoryPreviewCaptureActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PlayerStatsComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "World/WorldItemActor.h"
#include "Items/ItemData.h"
#include "Items/ItemSizeRules.h"
#include "Items/ItemTypes.h"
#include "Weapons/WeaponBase.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// ======================= LOCAL HELPER =======================
// Автовыбор слота для оружия (Weapon1/Weapon2) и девайсов (Device1/Device2)
static EEquipmentSlotType ResolveAutoSlot(
	EEquipmentSlotType DesiredSlot,
	const TMap<EEquipmentSlotType, FItemBaseRow>& CurrentEquip)
{
	// --- Группа основных пушек ---
	if (DesiredSlot == EEquipmentSlotType::EquipmentSlotWeapon1 ||
		DesiredSlot == EEquipmentSlotType::EquipmentSlotWeapon2)
	{
		const bool bHas1 = CurrentEquip.Contains(EEquipmentSlotType::EquipmentSlotWeapon1);
		const bool bHas2 = CurrentEquip.Contains(EEquipmentSlotType::EquipmentSlotWeapon2);

		if (!bHas1)
			return EEquipmentSlotType::EquipmentSlotWeapon1;

		if (!bHas2)
			return EEquipmentSlotType::EquipmentSlotWeapon2;

		// Оба заняты — пока просто возвращаем Weapon1
		// (позже можно сделать логика «выкинуть/переложить»)
		return EEquipmentSlotType::EquipmentSlotWeapon1;
	}

	// --- Группа девайсов ---
	if (DesiredSlot == EEquipmentSlotType::EquipmentSlotDevice1 ||
		DesiredSlot == EEquipmentSlotType::EquipmentSlotDevice2)
	{
		const bool bHas1 = CurrentEquip.Contains(EEquipmentSlotType::EquipmentSlotDevice1);
		const bool bHas2 = CurrentEquip.Contains(EEquipmentSlotType::EquipmentSlotDevice2);

		if (!bHas1)
			return EEquipmentSlotType::EquipmentSlotDevice1;

		if (!bHas2)
			return EEquipmentSlotType::EquipmentSlotDevice2;

		return EEquipmentSlotType::EquipmentSlotDevice1;
	}

	// Для остальных слотов ничего умного не делаем
	return DesiredSlot;
}

static EEquipmentSlotType GetHeroPartSlot(EHeroPartType HeroPartType)
{
	switch (HeroPartType)
	{
	case EHeroPartType::Head: return EEquipmentSlotType::EquipmentSlotHead;
	case EHeroPartType::Body: return EEquipmentSlotType::EquipmentSlotBody;
	case EHeroPartType::Legs: return EEquipmentSlotType::EquipmentSlotLegs;
	default:                  return EEquipmentSlotType::None;
	}
}

// ======================= INVENTORY HELPERS (Drag&Drop) =======================
static bool RectOverlapCells(
	int32 AX, int32 AY, const FItemSize& ASz,
	int32 BX, int32 BY, const FItemSize& BSz)
{
	const int32 AL = AX;
	const int32 AT = AY;
	const int32 AR = AX + ASz.Width  - 1;
	const int32 AB = AY + ASz.Height - 1;

	const int32 BL = BX;
	const int32 BT = BY;
	const int32 BR = BX + BSz.Width  - 1;
	const int32 BB = BY + BSz.Height - 1;

	return !(AR < BL || AL > BR || AB < BT || AT > BB);
}

static bool CanPlaceInInventory(
	const TArray<FInventoryItemEntry>& Items,
	int32 InvW, int32 InvH,
	const FItemSize& Size,
	int32 CellX, int32 CellY,
	int32 IgnoreIndex)
{
	if (Size.Width <= 0 || Size.Height <= 0)
		return false;

	if (CellX < 0 || CellY < 0)
		return false;

	if (CellX + Size.Width > InvW)
		return false;

	if (CellY + Size.Height > InvH)
		return false;

	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (i == IgnoreIndex)
			continue;

		const FInventoryItemEntry& E = Items[i];
		if (RectOverlapCells(CellX, CellY, Size, E.CellX, E.CellY, E.SizeInCells))
			return false;
	}

	return true;
}

static bool IsWeaponSlotType(EEquipmentSlotType Slot)
{
	return Slot == EEquipmentSlotType::EquipmentSlotWeapon1 || Slot == EEquipmentSlotType::EquipmentSlotWeapon2;
}

static bool IsDeviceSlotType(EEquipmentSlotType Slot)
{
	return Slot == EEquipmentSlotType::EquipmentSlotDevice1 || Slot == EEquipmentSlotType::EquipmentSlotDevice2;
}

static bool IsWeaponAffectingSlot(EEquipmentSlotType Slot)
{
	return Slot == EEquipmentSlotType::EquipmentSlotWeapon1 ||
		   Slot == EEquipmentSlotType::EquipmentSlotWeapon2 ||
		   Slot == EEquipmentSlotType::EquipmentSlotPistol ||
		   Slot == EEquipmentSlotType::EquipmentSlotKnife;
}

// ===== ContextMenu helpers =====
static int32 FindInventoryIndexForAction(const TArray<FInventoryItemEntry>& Items, FName InternalName, int32 CellX, int32 CellY)
{
	int32 Idx = Items.IndexOfByPredicate([&](const FInventoryItemEntry& E)
	{
		return E.ItemRow.InternalName == InternalName && E.CellX == CellX && E.CellY == CellY;
	});

	if (Idx != INDEX_NONE)
	{
		return Idx;
	}

	// fallback: только по InternalName (если вкладки уплотняются/координаты не те)
	return Items.IndexOfByPredicate([&](const FInventoryItemEntry& E)
	{
		return E.ItemRow.InternalName == InternalName;
	});
}

static int32 FindBySubCategory(const TArray<FInventoryItemEntry>& Items, EStoreSubCategory SubCat)
{
	return Items.IndexOfByPredicate([&](const FInventoryItemEntry& E)
	{
		return E.ItemRow.StoreSubCategory == SubCat;
	});
}

static int32 FindAnyAmmoIndex(const TArray<FInventoryItemEntry>& Items)
{
	return Items.IndexOfByPredicate([&](const FInventoryItemEntry& E)
	{
		return E.ItemRow.StoreCategory == EStoreCategory::storecat_Ammo;
	});
}

static int32 FindAmmoIndexBySubCategory(const TArray<FInventoryItemEntry>& Items, EStoreSubCategory AmmoSubCat)
{
	return Items.IndexOfByPredicate([&](const FInventoryItemEntry& E)
	{
		return (E.ItemRow.StoreCategory == EStoreCategory::storecat_Ammo) &&
			   (E.ItemRow.StoreSubCategory == AmmoSubCat);
	});
}

//////////////////////////////////////////////////////////////////////////
// AARESMMOCharacter
AARESMMOCharacter::AARESMMOCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->JumpZVelocity = 700.f;
		Move->AirControl = 0.35f;
		Move->MinAnalogWalkSpeed = 20.f;
		Move->BrakingDecelerationWalking = 2000.f;
		Move->BrakingDecelerationFalling = 1500.0f;

		// ВАЖНО: вместо жёсткого 500.f используем нашу переменную WalkSpeed
		Move->MaxWalkSpeed         = WalkSpeed;
		Move->MaxWalkSpeedCrouched = CrouchSpeed;

		Move->NavAgentProps.bCanCrouch = true;
		Move->CrouchedHalfHeight = 60.0f;
	}

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a TPS Camera
	TPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPSCamera"));
	TPSCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	TPSCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// ===== Create Modular Meshes =====
	Mesh_Hair = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Hair")); // Hair
	Mesh_Hair->SetupAttachment(GetMesh());
	Mesh_Hair->SetLeaderPoseComponent(GetMesh());

	Mesh_Beard = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Beard")); // Beard
	Mesh_Beard->SetupAttachment(GetMesh());
	Mesh_Beard->SetLeaderPoseComponent(GetMesh());
	
	Mesh_Head = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Head")); // Head
	Mesh_Head->SetupAttachment(GetMesh());
	Mesh_Head->SetLeaderPoseComponent(GetMesh());

	Mesh_Hand = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Hand")); // Hand
	Mesh_Hand->SetupAttachment(GetMesh());
	Mesh_Hand->SetLeaderPoseComponent(GetMesh());
	
	Mesh_Body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Body")); // Body
	Mesh_Body->SetupAttachment(GetMesh());
	Mesh_Body->SetLeaderPoseComponent(GetMesh());
	
	Mesh_Legs = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Legs")); // Legs
	Mesh_Legs->SetupAttachment(GetMesh());
	Mesh_Legs->SetLeaderPoseComponent(GetMesh());

	// ===== Create Equipment Meshes =====
	Mesh_Armor = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Armor"));
	Mesh_Armor->SetupAttachment(GetMesh());
	Mesh_Armor->SetLeaderPoseComponent(GetMesh());

	Mesh_Helmet = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Helmet"));
	Mesh_Helmet->SetupAttachment(GetMesh());
	Mesh_Helmet->SetLeaderPoseComponent(GetMesh());

	Mesh_Mask = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Mask"));
	Mesh_Mask->SetupAttachment(GetMesh());
	Mesh_Mask->SetLeaderPoseComponent(GetMesh());

	Mesh_Backpack = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Backpack"));
	Mesh_Backpack->SetupAttachment(GetMesh());
	Mesh_Backpack->SetLeaderPoseComponent(GetMesh());

	// Create a FPS Camera
	FPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCamera->SetupAttachment(GetMesh(), TEXT("head")); // имя сокета головы
	FPSCamera->bUsePawnControlRotation = true;

	if (TPSCamera)
	{
		TPSCamera->SetActive(true);
	}
	if (FPSCamera)
	{
		FPSCamera->SetActive(false);
	}
	bIsFirstPerson = false;

	// ===== Player StatsComponent =====
	Stats = CreateDefaultSubobject<UPlayerStatsComponent>(TEXT("PlayerStats"));

	// === SceneCapture для превью ===
	InventoryPreviewPivot = CreateDefaultSubobject<USceneComponent>(TEXT("InventoryPreviewPivot"));
	InventoryPreviewPivot->SetupAttachment(GetMesh());
	
	InventoryCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("InventoryCapture"));
	InventoryCapture->SetupAttachment(InventoryPreviewPivot);
	InventoryCapture->bCaptureEveryFrame = true;
	InventoryCapture->bCaptureOnMovement = false;
	InventoryCapture->FOVAngle = 35.f;

	// Позиция камеры "как в инвентаре"
	InventoryPreviewPivot->SetRelativeLocation(FVector::ZeroVector);
	InventoryPreviewPivot->SetRelativeRotation(FRotator::ZeroRotator);
	
	InventoryCapture->SetRelativeLocation(FVector(0.f, 150.f, 80.f));
	InventoryCapture->SetRelativeRotation(FRotator(0.f, -180.f, 0.f));
	InventoryCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
}

void AARESMMOCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Сохраняем дефолтные настройки TPS камеры
	if (CameraBoom)
	{
		TPSDefaultArmLength = CameraBoom->TargetArmLength;
	}

	if (TPSCamera)
	{
		TPSDefaultFOV = TPSCamera->FieldOfView;
	}

	// Создаём постоянный GameHUD только на локальном игроке
	if (IsLocallyControlled() && GameHUDWidgetClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			GameHUDWidgetInstance = CreateWidget<UGameHUDWidget>(PC, GameHUDWidgetClass);
			if (GameHUDWidgetInstance)
			{
				GameHUDWidgetInstance->AddToViewport();
				GameHUDWidgetInstance->InitFromPawn(this);
			}
		}
	}

	// Кешируем стандартные меши героя
	if (Mesh_Hair)
	{
		DefaultHairMesh = Mesh_Hair->GetSkeletalMeshAsset();
	}
	if (Mesh_Beard)
	{
		DefaultBeardMesh = Mesh_Beard->GetSkeletalMeshAsset();
	}
	if (Mesh_Head)
	{
		DefaultHeadMesh = Mesh_Head->GetSkeletalMeshAsset();
	}
	if (Mesh_Hand)
	{
		DefaultHandMesh = Mesh_Hand->GetSkeletalMeshAsset();
	}
	if (Mesh_Body)
	{
		DefaultBodyMesh = Mesh_Body->GetSkeletalMeshAsset();
	}
	if (Mesh_Legs)
	{
		DefaultLegsMesh = Mesh_Legs->GetSkeletalMeshAsset();
	}

	// =====================================================================
	// Inventory Preview Capture (ОТДЕЛЬНЫЙ актёр, чтобы FPS-скрытие не влияло)
	// =====================================================================
	if (IsLocallyControlled() && InventoryRenderTarget)
	{
		// Спавним актёр превью (SceneCapture живёт НЕ на персонаже)
		if (!InventoryPreviewActor)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			Params.Owner = this;

			InventoryPreviewActor = GetWorld()->SpawnActor<AInventoryPreviewCaptureActor>(
				AInventoryPreviewCaptureActor::StaticClass(),
				FTransform::Identity,
				Params
			);

			if (InventoryPreviewActor)
			{
				InventoryPreviewActor->Init(this, InventoryRenderTarget);
			}
		}

		// Старый InventoryCapture на персонаже выключаем, чтобы не было двойного захвата
		if (InventoryCapture)
		{
			InventoryCapture->bCaptureEveryFrame = false;
			InventoryCapture->TextureTarget = nullptr;
		}
	}

	// Собираем ShowOnly (и оружие тоже) уже через UpdateInventoryPreviewShowOnly()
	UpdateInventoryPreviewShowOnly();
	SetSelectedWeaponInternal(GetBestWeaponForAttachment());
}

void AARESMMOCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InventoryPreviewActor)
	{
		InventoryPreviewActor->Destroy();
		InventoryPreviewActor = nullptr;
	}

	if (GameHUDWidgetInstance)
	{
		GameHUDWidgetInstance->RemoveFromParent();
		GameHUDWidgetInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AARESMMOCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateAnimMovementData(DeltaSeconds);

	if (!Stats)
	{
		return;
	}

	// Если вымотан — стоим, пока стамина не восстановится до 15%
	if (bIsExhausted)
	{
		if (Stats->Base.Stamina >= MinStaminaPercentToSprint)
		{
			SetExhausted(false);
		}
		return;
	}

	// --------- Стамина от спринта ---------
	if (bIsSprinting)
	{
		Stats->ConsumeStamina(SprintStaminaCostPerSecond * DeltaSeconds);

		if (Stats->Base.Stamina <= 0.f)
		{
			// В 0 — стоп и блок движения до 15%
			StopSprint();
			SetExhausted(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AARESMMOCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AARESMMOCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Movement
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AARESMMOCharacter::Move); // Walk
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &AARESMMOCharacter::Sprinting); // Sprinting
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AARESMMOCharacter::Crouching); // Crouching
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AARESMMOCharacter::Look);

		if (ToggleViewAction) // Смена камеры
		{
			EnhancedInputComponent->BindAction(ToggleViewAction, ETriggerEvent::Started, this, &AARESMMOCharacter::ToggleCameraMode);
		}

		if (InventoryAction) // Открытие/Закрытие Инвентаря
		{
			EnhancedInputComponent->BindAction(
				InventoryAction,
				ETriggerEvent::Started,
				this,
				&AARESMMOCharacter::ToggleInventory
			);
		}

		// Aim Offset
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(
				AimAction,
				ETriggerEvent::Started,
				this,
				&AARESMMOCharacter::StartAim
			);

			EnhancedInputComponent->BindAction(
				AimAction,
				ETriggerEvent::Completed,
				this,
				&AARESMMOCharacter::StopAim
			);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AARESMMOCharacter::UpdateAnimMovementData(float DeltaSeconds)
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	// ===== Velocity / GroundSpeed (XY only) =====
	Velocity = Move->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.0f).Size();

	// ===== Acceleration / Falling / Crouch =====
	Acceleration = Move->GetCurrentAcceleration();
	bIsFalling = Move->IsFalling();
	bIsCrouching = Move->IsCrouching();

	// ===== ShouldMove =====
	const bool bSpeedOk = (GroundSpeed > 3.0f);
	const bool bHasAccel = !Acceleration.IsNearlyZero(1.0f);
	bShouldMove = bSpeedOk && bHasAccel;

	// ===== Direction (как CalculateDirection + NormalizeAxis) =====
	const FVector Vel2D = FVector(Velocity.X, Velocity.Y, 0.0f);
	if (Vel2D.SizeSquared() > FMath::Square(3.0f))
	{
		const FRotator BaseRot(0.0f, GetActorRotation().Yaw, 0.0f);
		const FVector Forward = FRotationMatrix(BaseRot).GetUnitAxis(EAxis::X);
		const FVector Right   = FRotationMatrix(BaseRot).GetUnitAxis(EAxis::Y);

		const float ForwardVel = FVector::DotProduct(Vel2D, Forward);
		const float RightVel   = FVector::DotProduct(Vel2D, Right);

		Direction = FMath::UnwindDegrees(FMath::RadiansToDegrees(FMath::Atan2(RightVel, ForwardVel)));
	}
	else
	{
		Direction = 0.0f;
	}

	// ===== MoveDirection (Client_MovementState) =====
	if (!bSpeedOk)
	{
		MoveDirection = EMoveDirection::None;
	}
	else if (Direction >= -70.0f && Direction <= 70.0f)
	{
		MoveDirection = EMoveDirection::Forward;
	}
	else if (Direction >= 70.0f && Direction <= 110.0f)
	{
		MoveDirection = EMoveDirection::Right;
	}
	else if (Direction >= -110.0f && Direction <= -70.0f)
	{
		MoveDirection = EMoveDirection::Left;
	}
	else
	{
		MoveDirection = EMoveDirection::Backward;
	}

	// ===== DirectionAngle =====
	DirectionAngle = FMath::FInterpTo(DirectionAngle, TurnRate, DeltaSeconds, 10.0f);

	// ===== Orientation Angles for Orientation Warping =====
	F_OrientationAngle = FMath::UnwindDegrees(Direction - 0.0f);
	R_OrientationAngle = FMath::UnwindDegrees(Direction - 90.0f);
	B_OrientationAngle = FMath::UnwindDegrees(Direction - 180.0f);
	L_OrientationAngle = FMath::UnwindDegrees(Direction - (-90.0f));
}

void AARESMMOCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection,   MovementVector.X);
	}
}

void AARESMMOCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (!Controller) return;

	// сигнал для AnimBP
	TurnRate = FMath::Clamp(LookAxisVector.X * TurnRateScale, -1.0f, 1.0f);

	// сама камера — без clamp
	AddControllerYawInput(LookAxisVector.X * MouseYawSensitivity);
	AddControllerPitchInput(LookAxisVector.Y * MousePitchSensitivity);
}

void AARESMMOCharacter::Sprinting(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();

	if (bPressed)
	{
		StartSprint();
	}
	else
	{
		StopSprint();
	}
}

void AARESMMOCharacter::Crouching(const FInputActionValue& Value)
{
	const bool bPressed = Value.Get<bool>();
	if (!bPressed)
	{
		// На Started достаточно реагировать только на true
		return;
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// Уже сидим → встаём
		if (Move->IsCrouching())
		{
			UnCrouch();

			// если до этого был спринт — вернём спринтовую скорость
			if (bIsSprinting)
			{
				Move->MaxWalkSpeed = SprintSpeed;
			}
			else
			{
				Move->MaxWalkSpeed = WalkSpeed;
			}
		}
		else
		{
			// Входим в присед: спринт гасим, скорость = CrouchSpeed
			StopSprint();           // сбросит bIsSprinting и WalkSpeed
			Crouch();               // стандартный Character::Crouch()
			
			if (Move)
			{
				Move->MaxWalkSpeed = CrouchSpeed;
			}
		}
	}
}

bool AARESMMOCharacter::DetachWeaponATTMToInventory(EStoreSubCategory SubCategory)
{
	AWeaponBase* Weapon = GetSelectedWeapon();
	if (!Weapon)
	{
		// fallback на приоритет Weapon1->Weapon2->Pistol (твоя внутренняя логика)
		Weapon = GetBestWeaponForAttachment();
	}

	if (!Weapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("DetachWeaponATTMToInventory: No weapon to detach from."));
		return false;
	}

	FItemBaseRow DetachedRow;
	FString FailReason;
	if (!Weapon->DetachItem(SubCategory, DetachedRow, FailReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("DetachWeaponATTMToInventory: detach failed: %s | Slot=%d"),
			*FailReason, (int32)SubCategory);
		return false;
	}

	// вернуть предмет в инвентарь
	if (!AddItemToInventory(DetachedRow, 1))
	{
		// попытка отката (чтобы не потерять предмет)
		Weapon->AttachItem(DetachedRow);

		UE_LOG(LogTemp, Warning, TEXT("DetachWeaponATTMToInventory: inventory full, rollback attach. | Item=%s"),
			*DetachedRow.InternalName.ToString());
		return false;
	}

	RefreshInventoryUI();
	return true;
}

void AARESMMOCharacter::SelectWeaponSlot(EEquipmentSlotType SlotType)
{
	AWeaponBase* W = GetWeaponActorInSlot(SlotType);
	SetSelectedWeaponInternal(W);
}

void AARESMMOCharacter::EquipHeroPart(const FItemBaseRow& ItemRow)
{
	if (ItemRow.HeroPartType == EHeroPartType::None)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipHeroPart: Invalid item type."));
		return;
	}

	switch (ItemRow.HeroPartType)
	{
	case EHeroPartType::Head:
		if (Mesh_Head)
		{
			Mesh_Head->SetSkeletalMesh(ItemRow.HeroPartMesh);
		}
		break;

	case EHeroPartType::Body:
		if (Mesh_Body)
		{
			Mesh_Body->SetSkeletalMesh(ItemRow.HeroPartMesh);
		}
		break;

	case EHeroPartType::Legs:
		if (Mesh_Legs)
		{
			Mesh_Legs->SetSkeletalMesh(ItemRow.HeroPartMesh);
		}
		break;
	default: ;
	}

	if (!ItemRow.HeroPartMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipHeroPart: Mesh is not assigned for item %s"),
				*ItemRow.InternalName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Equipped mesh for part: %d"), (int)ItemRow.HeroPartType);
	}
}

void AARESMMOCharacter::EquipEquipment(const FItemBaseRow& ItemRow)
{
	switch (ItemRow.StoreCategory)
	{
	case EStoreCategory::storecat_Armor:
		if (Mesh_Armor)
		{
			Mesh_Armor->SetSkeletalMesh(ItemRow.HeroPartMesh);
		}
		break;

	case EStoreCategory::storecat_Helmet:
		if (Mesh_Helmet)
		{
			Mesh_Helmet->SetSkeletalMesh(ItemRow.HeroPartMesh);
		}
		break;

	case EStoreCategory::storecat_Mask:
		if (Mesh_Mask)
		{
			Mesh_Mask->SetSkeletalMesh(ItemRow.HeroPartMesh);
		}
		break;

	case EStoreCategory::storecat_Backpack:
		if (Mesh_Backpack)
		{
			Mesh_Backpack->SetSkeletalMesh(ItemRow.HeroPartMesh);
		}
		break;

	default:
		if (!ItemRow.HeroPartMesh)
		{
			UE_LOG(LogTemp, Warning, TEXT("EquipEquipment: Invalid mesh for item %s"),
					*ItemRow.InternalName.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Equipped equipment: %s"),
					   *ItemRow.InternalName.ToString());
		}
	}
}

void AARESMMOCharacter::EquipItem(const FItemBaseRow& ItemRow)
{
	const bool bIsHeroPart =
		(ItemRow.StoreCategory == EStoreCategory::storecat_HeroParts);

	const bool bIsGear =
		(ItemRow.StoreCategory == EStoreCategory::storecat_Armor  ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_Helmet ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_Mask   ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_Backpack);

	// Визуал на персонаже (HeroParts/Gear) + WeaponState
	if (bIsHeroPart)
	{
		EquipHeroPart(ItemRow);
	}
	else if (bIsGear)
	{
		EquipEquipment(ItemRow);
	}
	else
	{
		// оружие/девайсы → меняем WeaponState
		const EWeaponState NewState = GetWeaponStateForCategory(ItemRow.StoreCategory);
		if (NewState != EWeaponState::Unarmed)
		{
			SetWeaponState(NewState);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("EquipItem: Category %d does not change weapon state"),
				static_cast<int32>(ItemRow.StoreCategory));
		}
	}

	// Определяем слот экипировки, чтобы UI видел предмет
	EEquipmentSlotType Slot = ItemRow.EquipmentSlot;

	if (Slot == EEquipmentSlotType::None)
	{
		Slot = GetEquipmentSlotForCategory(ItemRow.StoreCategory);

		// Особый случай: HeroParts (голова/туловище/ноги)
		if (Slot == EEquipmentSlotType::None && bIsHeroPart)
		{
			switch (ItemRow.HeroPartType)
			{
			case EHeroPartType::Head: Slot = EEquipmentSlotType::EquipmentSlotHead; break;
			case EHeroPartType::Body: Slot = EEquipmentSlotType::EquipmentSlotBody; break;
			case EHeroPartType::Legs: Slot = EEquipmentSlotType::EquipmentSlotLegs; break;
			default: break;
			}
		}
	}

	// ResolveAutoSlot — чтобы, например, Armor ушёл в Armor, Weapon1/2 и т.д.
	Slot = ResolveAutoSlot(Slot, EquipmentSlots);

	if (Slot != EEquipmentSlotType::None)
	{
		EquipmentSlots.FindOrAdd(Slot) = ItemRow;

		// обновляем панель экипировки
		if (InventoryLayoutWidgetInstance)
		{
			InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
		}
	}
	
	const bool bIsWeaponSlot =
		(Slot == EEquipmentSlotType::EquipmentSlotPistol ||
		 Slot == EEquipmentSlotType::EquipmentSlotWeapon1 ||
		 Slot == EEquipmentSlotType::EquipmentSlotWeapon2);

	if (bIsWeaponSlot)
	{
		// Спавн/Attach Actor оружия (BP_Weapon_* должен быть наследником AWeaponBase)
		EquipWeaponActorToSlot(ItemRow, Slot);
	}
}

bool AARESMMOCharacter::EquipItemFromInventory(const FItemBaseRow& ItemRow)
{
	// Ищем предмет в инвентаре (по InternalName)
	int32 FoundIndex = InventoryItems.IndexOfByPredicate(
		[&ItemRow](const FInventoryItemEntry& Entry)
		{
			return Entry.ItemRow.InternalName == ItemRow.InternalName;
		}
	);

	if (FoundIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipItemFromInventory: item %s not found in inventory"),
			*ItemRow.InternalName.ToString());
		return false;
	}

	// ЭКИПИРУЕМ — функция ничего не возвращает
	EquipItem(ItemRow);

	// Убираем предмет из инвентаря
	InventoryItems.RemoveAt(FoundIndex);

	// Обновляем UI, если инвентарь открыт
	if (InventoryLayoutWidgetInstance)
	{
		InventoryLayoutWidgetInstance->DistributeItems(InventoryItems);
		InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
	}

	return true;
}

bool AARESMMOCharacter::UnequipSlot(EEquipmentSlotType SlotType)
{
	FItemBaseRow RemovedItem;

	if (FItemBaseRow* Found = EquipmentSlots.Find(SlotType))
	{
		RemovedItem = *Found;
		EquipmentSlots.Remove(SlotType);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UnequipSlot: Slot %d is empty"), static_cast<int32>(SlotType));
		return false;
	}

	// Сбрасываем визуал, если нужно
	switch (SlotType)
	{
	case EEquipmentSlotType::EquipmentSlotHelmet:
		if (Mesh_Helmet)   Mesh_Helmet->SetSkeletalMesh(nullptr);
		break;

	case EEquipmentSlotType::EquipmentSlotArmor:
		if (Mesh_Armor)    Mesh_Armor->SetSkeletalMesh(nullptr);
		break;

	case EEquipmentSlotType::EquipmentSlotMask:
		if (Mesh_Mask)     Mesh_Mask->SetSkeletalMesh(nullptr);
		break;

	case EEquipmentSlotType::EquipmentSlotBackpack:
		if (Mesh_Backpack) Mesh_Backpack->SetSkeletalMesh(nullptr);
		break;

	case EEquipmentSlotType::EquipmentSlotHead:
		if (Mesh_Head) Mesh_Head->SetSkeletalMesh(DefaultHeadMesh);
		break;

	case EEquipmentSlotType::EquipmentSlotBody:
		if (Mesh_Body) Mesh_Body->SetSkeletalMesh(DefaultBodyMesh);
		break;

	case EEquipmentSlotType::EquipmentSlotLegs:
		if (Mesh_Legs) Mesh_Legs->SetSkeletalMesh(DefaultLegsMesh);
		break;
	
	case EEquipmentSlotType::EquipmentSlotPistol:
	{
		DestroyWeaponActorInSlot(EEquipmentSlotType::EquipmentSlotPistol);
	}
	
	case EEquipmentSlotType::EquipmentSlotWeapon1:
	case EEquipmentSlotType::EquipmentSlotWeapon2:
	{
		DestroyWeaponActorInSlot(SlotType);
	}

	default:
		break;
	}

	// Вернуть стойку после снятия Weapon1/Weapon2/Pistol/Knife
	if (SlotType == EEquipmentSlotType::EquipmentSlotPistol ||
		SlotType == EEquipmentSlotType::EquipmentSlotWeapon1 ||
		SlotType == EEquipmentSlotType::EquipmentSlotWeapon2 ||
		SlotType == EEquipmentSlotType::EquipmentSlotKnife)
	{
		RecalculateWeaponStateFromEquipment();
	}

	// Пытаемся вернуть предмет в инвентарь
	const bool bAdded = AddItemToInventory(RemovedItem, 1);
	if (!bAdded)
	{
		// нет места — откатываем (чтоб не терять предмет)
		EquipmentSlots.Add(SlotType, RemovedItem);

		// ВАЖНО: откатываем визуал БЕЗ EquipItem() (чтобы не уехал слот через ResolveAutoSlot)
		const bool bIsHeroPart = (RemovedItem.StoreCategory == EStoreCategory::storecat_HeroParts);
		const bool bIsGear =
			(RemovedItem.StoreCategory == EStoreCategory::storecat_Armor  ||
			 RemovedItem.StoreCategory == EStoreCategory::storecat_Helmet ||
			 RemovedItem.StoreCategory == EStoreCategory::storecat_Mask   ||
			 RemovedItem.StoreCategory == EStoreCategory::storecat_Backpack);

		if (bIsHeroPart)
		{
			EquipHeroPart(RemovedItem);
		}
		else if (bIsGear)
		{
			EquipEquipment(RemovedItem);
		}
		else
		{
			// если это оружейный слот — восстанавливаем actor-оружие
			if (SlotType == EEquipmentSlotType::EquipmentSlotPistol ||
				SlotType == EEquipmentSlotType::EquipmentSlotWeapon1 ||
				SlotType == EEquipmentSlotType::EquipmentSlotWeapon2)
			{
				EquipWeaponActorToSlot(RemovedItem, SlotType);
			}
		}

		RecalculateWeaponStateFromEquipment();

		UE_LOG(LogTemp, Warning, TEXT("UnequipSlot: no space in inventory for %s, reverting"),
			*RemovedItem.InternalName.ToString());

		// UI
		if (InventoryLayoutWidgetInstance)
		{
			InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
		}

		return false;
	}

	// UI
	if (InventoryLayoutWidgetInstance)
	{
		InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
	}

	return true;
}

void AARESMMOCharacter::UseItem(const FItemBaseRow& ItemRow)
{ // Заглушка: временно для использования предметов Medicine/Food/Water
	if (!Stats)
	{
		return;
	}

	switch (ItemRow.ItemClass)
	{
	case EItemClass::Medicine:
	case EItemClass::Food:
	case EItemClass::Water:
		Stats->ApplyConsumable(ItemRow);
		break;

	default:
		// другие классы (Ammo, Weapon, и т.п.) здесь не обрабатываем
		break;
	}
}

void AARESMMOCharacter::StartAim()
{
	// Aim работает ТОЛЬКО в TPS
	if (bIsFirstPerson)
	{
		return;
	}

	bIsAiming = true;

	// При Aim отключаем спринт
	bIsSprinting = false;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeed;
	}

	// Зумим TPS-камеру
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = TPSAimArmLength;
	}

	if (TPSCamera)
	{
		TPSCamera->SetFieldOfView(TPSAimFOV);
	}
}

void AARESMMOCharacter::StopAim()
{
	bIsAiming = false;

	// Возвращаем дефолтные значения TPS камеры
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = TPSDefaultArmLength;
	}

	if (TPSCamera)
	{
		TPSCamera->SetFieldOfView(TPSDefaultFOV);
	}
}

void AARESMMOCharacter::SetWeaponState(EWeaponState NewState)
{
	if (WeaponState == NewState)
	{
		return;
	}

	WeaponState = NewState;

	UE_LOG(LogTemp, Log, TEXT("WeaponState set to %d"), static_cast<int32>(WeaponState));
}

EWeaponState AARESMMOCharacter::GetWeaponStateForCategory(EStoreCategory Category)
{
	return ::GetWeaponStateForCategory(Category);
}

void AARESMMOCharacter::SetExhausted(bool bNewExhausted)
{
	if (bIsExhausted == bNewExhausted)
	{
		return;
	}

	bIsExhausted = bNewExhausted;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();

		if (bIsExhausted)
		{
			// Спринт гасим и фиксируем скорость в 0 
			bIsSprinting = false;
			Move->MaxWalkSpeed = ExhaustedMaxWalkSpeed;
		}
		else
		{
			// Возврат к нормальной скорости
			const bool bIsCrouchedNow = Move->IsCrouching();
			Move->MaxWalkSpeed = bIsCrouchedNow ? CrouchSpeed : WalkSpeed;
		}
	}
}

void AARESMMOCharacter::StartSprint()
{
	if (bIsExhausted)
	{
		return;
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->IsCrouching())
		{
			return;
		}

		// Проверка статов
		if (Stats)
		{
			// пока стамина < 15% — спринт запрещён
			if (Stats->Base.Stamina < MinStaminaPercentToSprint)
			{
				return;
			}

			// если жажда >= 60% — не бежать
			if (Stats->Secondary.Water >= 60.f)
			{
				return;
			}
		}

		bIsSprinting = true;
		Move->MaxWalkSpeed = SprintSpeed;
	}
}

void AARESMMOCharacter::StopSprint()
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		bIsSprinting = false;

		// если вымотан — оставляем скорость 0 (или ExhaustedMaxWalkSpeed)
		if (bIsExhausted)
		{
			Move->MaxWalkSpeed = ExhaustedMaxWalkSpeed;
			return;
		}

		// возвращаем обычную скорость (walk или crouch)
		const bool bIsCrouchedNow = Move->IsCrouching();
		Move->MaxWalkSpeed = bIsCrouchedNow ? CrouchSpeed : WalkSpeed;
	}
}

void AARESMMOCharacter::SwitchToFPS()
{
	if (!FPSCamera || !TPSCamera) return;

	StopAim();

	bIsFirstPerson = true;

	// --- Камеры ---
	TPSCamera->SetActive(false);
	FPSCamera->SetActive(true);

	// --- Вращение для FPS ---
	// В FPS UseControllerRotationYaw выключен
	bUseControllerRotationYaw = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// OrientRotationToMovement всегда OFF
		Move->bOrientRotationToMovement = true;
	}

	// --- Скрываем шмотку на голове в FPS ---
	auto HideForOwner = [](USkeletalMeshComponent* Comp, bool bHide)
	{
		if (!Comp) return;

		// компонент должен быть ВИДИМ глобально (для превью/других игроков)
		Comp->SetVisibility(true, true);

		// но скрыт только для владельца камеры
		Comp->SetOwnerNoSee(bHide);
		Comp->SetOnlyOwnerSee(false);
	};

	// --- Скрываем шмотку на голове только для камеры владельца (FPS) ---
	HideForOwner(Mesh_Hair, true);
	HideForOwner(Mesh_Beard, true);
	HideForOwner(Mesh_Helmet, true);
	HideForOwner(Mesh_Mask, true);
}

void AARESMMOCharacter::SwitchToTPS()
{
	bIsFirstPerson = false;

	// Выравниваем актёра под контроллер, чтобы не было перекоса
	if (AController* C = GetController())
	{
		FRotator NewActorRot = C->GetControlRotation();
		NewActorRot.Pitch = 0.f;
		NewActorRot.Roll  = 0.f;
		SetActorRotation(NewActorRot);
	}

	// В TPS включаем UseControllerRotationYaw
	bUseControllerRotationYaw = true;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// OrientRotationToMovement всегда OFF
		Move->bOrientRotationToMovement = true;
	}
	
	FPSCamera->SetActive(false);
	TPSCamera->SetActive(true);

	// --- Отображаем шмотку на голове в TPS ---
	auto HideForOwner = [](USkeletalMeshComponent* Comp, bool bHide)
	{
		if (!Comp) return;

		// компонент должен быть ВИДИМ глобально (для превью/других игроков)
		Comp->SetVisibility(true, true);

		// но скрыт только для владельца камеры
		Comp->SetOwnerNoSee(bHide);
		Comp->SetOnlyOwnerSee(false);
	};

	// --- Возвращаем видимость для владельца (TPS) ---
	HideForOwner(Mesh_Hair, false);
	HideForOwner(Mesh_Beard, false);
	HideForOwner(Mesh_Helmet, false);
	HideForOwner(Mesh_Mask, false);
}

void AARESMMOCharacter::ToggleCameraMode()
{
	if (bIsFirstPerson)
	{
		SwitchToTPS();
	}
	else
	{
		SwitchToFPS();
	}
}

void AARESMMOCharacter::ToggleInventory()
{
	if (!InventoryLayoutWidgetInstance)
	{
		if (!InventoryLayoutWidgetClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("InventoryLayoutWidgetClass is not set on %s"), *GetName());
			return;
		}

		APlayerController* PC = Cast<APlayerController>(GetController());
		if (!PC)
		{
			return;
		}

		InventoryLayoutWidgetInstance = CreateWidget<UInventoryLayoutWidget>(PC, InventoryLayoutWidgetClass);
		if (InventoryLayoutWidgetInstance)
		{
			InventoryLayoutWidgetInstance->AddToViewport();
			InventoryLayoutWidgetInstance->InitPreview(this);

			// ПЕРЕДАЁМ ТЕКУЩИЙ ИНВЕНТАРЬ
			InventoryLayoutWidgetInstance->DistributeItems(InventoryItems);
			// ПЕРЕДАЁМ ТЕКУЩИЕ СЛОТЫ ЭКИПИРОВКИ
			InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);

			bIsInventoryOpen = true;
		}
	}
    else
    {
        const bool bNewOpen = !bIsInventoryOpen;

        InventoryLayoutWidgetInstance->SetVisibility(
            bNewOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
        );

        // при каждом открытии обновляем превью (на случай новой "эквип")
    	if (bNewOpen)
    	{
    		InventoryLayoutWidgetInstance->InitPreview(this);
    		InventoryLayoutWidgetInstance->DistributeItems(InventoryItems);
    		InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
    	}

        bIsInventoryOpen = bNewOpen;
    }

    // ----- Режим ввода и курсор -----
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (bIsInventoryOpen)
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(InventoryLayoutWidgetInstance->TakeWidget());
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(InputMode);

            PC->bShowMouseCursor = true;
            PC->SetIgnoreLookInput(true);
            PC->SetIgnoreMoveInput(true);
        }
        else
        {
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);

            PC->bShowMouseCursor = false;
            PC->SetIgnoreLookInput(false);
            PC->SetIgnoreMoveInput(false);
        }
    }
}

void AARESMMOCharacter::RecalculateWeaponStateFromEquipment()
{
	auto TryGetStateFromSlot = [&](EEquipmentSlotType Slot, EWeaponState& OutState) -> bool
	{
		if (const FItemBaseRow* Row = EquipmentSlots.Find(Slot))
		{
			const EWeaponState S = GetWeaponStateForCategory(Row->StoreCategory);
			if (S != EWeaponState::Unarmed)
			{
				OutState = S;
				return true;
			}
		}
		return false;
	};

	EWeaponState NewState = EWeaponState::Unarmed;

	// Приоритет:
	// Основные стволы
	if (TryGetStateFromSlot(EEquipmentSlotType::EquipmentSlotWeapon1, NewState) ||
		TryGetStateFromSlot(EEquipmentSlotType::EquipmentSlotWeapon2, NewState) ||
		// Для Пистолетов
		TryGetStateFromSlot(EEquipmentSlotType::EquipmentSlotPistol, NewState) ||
		// Для Melee
		TryGetStateFromSlot(EEquipmentSlotType::EquipmentSlotKnife, NewState))
	{
		SetWeaponState(NewState);
		return;
	}

	// Ничего оружейного не осталось
	SetWeaponState(EWeaponState::Unarmed);
}

AWeaponBase* AARESMMOCharacter::GetWeaponActorInSlot(EEquipmentSlotType SlotType) const
{
	switch (SlotType)
	{
		case EEquipmentSlotType::EquipmentSlotWeapon1: return WeaponActor_Weapon1;
		case EEquipmentSlotType::EquipmentSlotWeapon2: return WeaponActor_Weapon2;
		case EEquipmentSlotType::EquipmentSlotPistol:  return WeaponActor_Pistol;

		default: return nullptr;
	}
}

void AARESMMOCharacter::SetWeaponActorInSlot(EEquipmentSlotType SlotType, AWeaponBase* Weapon)
{
	switch (SlotType)
	{
		case EEquipmentSlotType::EquipmentSlotWeapon1: WeaponActor_Weapon1 = Weapon; break;
		case EEquipmentSlotType::EquipmentSlotWeapon2: WeaponActor_Weapon2 = Weapon; break;
		case EEquipmentSlotType::EquipmentSlotPistol:  WeaponActor_Pistol  = Weapon; break;

		default: break;
	}
}

bool AARESMMOCharacter::EquipWeaponActorToSlot(const FItemBaseRow& ItemRow, EEquipmentSlotType SlotType)
{
	if (!GetWorld() || !GetMesh())
		return false;

	// Если в DT не задан класс — это не actor-оружие
	if (ItemRow.WeaponActorClass.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipWeaponActorToSlot: WeaponActorClass is null for %s"), *ItemRow.InternalName.ToString());
		return false;
	}

	// Сносим старое оружие в слоте
	DestroyWeaponActorInSlot(SlotType);

	UClass* WeaponCls = ItemRow.WeaponActorClass.LoadSynchronous();
	if (!WeaponCls)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipWeaponActorToSlot: failed to load class for %s"), *ItemRow.InternalName.ToString());
		return false;
	}

	FActorSpawnParameters SP;
	SP.Owner = this;
	SP.Instigator = this;
	SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponCls, FTransform::Identity, SP);
	if (!NewWeapon)
		return false;

	NewWeapon->SetOwningCharacter(this);

	FName AttachName = NewWeapon->GetCharacterAttachSocket();
	if (AttachName.IsNone())
	{
		AttachName = TEXT("weapon_r");
	}

	// Если сокета нет на персонаже — жёсткий fallback на weapon_r
	if (!GetMesh()->DoesSocketExist(AttachName))
	{
		AttachName = TEXT("weapon_r");
	}

	NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachName);

	SetWeaponActorInSlot(SlotType, NewWeapon);

	// Если активное оружие ещё не выбрано — выбираем первое появившееся
	if (!SelectedWeapon && NewWeapon)
	{
		SetSelectedWeaponInternal(NewWeapon);
	}

	UpdateInventoryPreviewShowOnly();
	return true;
}

void AARESMMOCharacter::DestroyWeaponActorInSlot(EEquipmentSlotType SlotType)
{
	if (AWeaponBase* W = GetWeaponActorInSlot(SlotType))
	{
		const bool bWasSelected = (SelectedWeapon == W);

		W->Destroy();
		SetWeaponActorInSlot(SlotType, nullptr);

		if (bWasSelected)
		{
			SetSelectedWeaponInternal(GetBestWeaponForAttachment()); // Weapon1->Weapon2->Pistol или nullptr
		}
	}

	UpdateInventoryPreviewShowOnly();
}

void AARESMMOCharacter::UpdateInventoryPreviewShowOnly()
{
	if (!InventoryRenderTarget)
	{
		return;
	}

	USceneCaptureComponent2D* Capture = nullptr;

	if (InventoryPreviewActor && InventoryPreviewActor->GetCaptureComponent())
	{
		Capture = InventoryPreviewActor->GetCaptureComponent();
	}
	else
	{
		Capture = InventoryCapture; // fallback
	}

	if (!Capture)
	{
		return;
	}

	Capture->TextureTarget = InventoryRenderTarget;
	Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	Capture->ShowOnlyComponents.Reset();

	// все компоненты персонажа
	Capture->ShowOnlyActorComponents(this, true);

	// добавляем все компоненты оружия (включая прицел/рукоятку и т.д.)
	if (WeaponActor_Weapon1) InventoryCapture->ShowOnlyActorComponents(WeaponActor_Weapon1, true);
	if (WeaponActor_Weapon2) InventoryCapture->ShowOnlyActorComponents(WeaponActor_Weapon2, true);
	if (WeaponActor_Pistol)  InventoryCapture->ShowOnlyActorComponents(WeaponActor_Pistol,  true);
}

AWeaponBase* AARESMMOCharacter::GetBestWeaponForAttachment() const
{
	if (WeaponActor_Weapon1) return WeaponActor_Weapon1;
	if (WeaponActor_Weapon2) return WeaponActor_Weapon2;
	if (WeaponActor_Pistol)  return WeaponActor_Pistol;
	return nullptr;
}

void AARESMMOCharacter::SetSelectedWeaponInternal(AWeaponBase* NewWeapon)
{
	SelectedWeapon = NewWeapon;
	Hands_IK_Weight = (SelectedWeapon != nullptr) ? 1.0f : 0.0f;
}

void AARESMMOCharacter::AddInventoryPreviewYaw(float DeltaYaw)
{
	if (!InventoryPreviewPivot) return;

	FRotator Rot = InventoryPreviewPivot->GetRelativeRotation();
	Rot.Yaw += DeltaYaw * 0.5f;        // скорость вращения
	InventoryPreviewPivot->SetRelativeRotation(Rot);
}

bool AARESMMOCharacter::AddItemToInventory(const FItemBaseRow& ItemRow, int32 StackCount)
{
	if (InventoryWidthCells <= 0 || InventoryHeightCells <= 0)
	{
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("AddItemToInventory: %s"), *ItemRow.InternalName.ToString());

	FInventoryItemEntry NewEntry;
	NewEntry.ItemRow     = ItemRow;
	NewEntry.SizeInCells = ItemRow.GridSize;

	// fallback если вдруг в DataTable выставили 0x0
	if (NewEntry.SizeInCells.Width <= 0 || NewEntry.SizeInCells.Height <= 0)
	{
		NewEntry.SizeInCells = UItemSizeRules::GetDefaultSize(ItemRow.ItemClass);
	}

	const int32 ItemW = NewEntry.SizeInCells.Width;
	const int32 ItemH = NewEntry.SizeInCells.Height;

	if (ItemW <= 0 || ItemH <= 0)
	{
		return false;
	}

	const int32 InvW = InventoryWidthCells;
	const int32 InvH = InventoryHeightCells;

	TArray<bool> Occupied;
	Occupied.Init(false, InvW * InvH);

	auto MarkOccupied = [&](const FInventoryItemEntry& E)
	{
		for (int32 dx = 0; dx < E.SizeInCells.Width; ++dx)
		{
			for (int32 dy = 0; dy < E.SizeInCells.Height; ++dy)
			{
				const int32 X = E.CellX + dx;
				const int32 Y = E.CellY + dy;

				if (X < 0 || X >= InvW || Y < 0 || Y >= InvH)
				{
					continue;
				}

				const int32 Index = Y * InvW + X;
				Occupied[Index] = true;
			}
		}
	};

	for (const FInventoryItemEntry& E : InventoryItems)
	{
		MarkOccupied(E);
	}

	const int32 MaxX = InvW - ItemW;
	const int32 MaxY = InvH - ItemH;

	for (int32 Y = 0; Y <= MaxY; ++Y)
	{
		for (int32 X = 0; X <= MaxX; ++X)
		{
			bool bCanPlace = true;

			for (int32 dx = 0; dx < ItemW && bCanPlace; ++dx)
			{
				for (int32 dy = 0; dy < ItemH && bCanPlace; ++dy)
				{
					const int32 TestX = X + dx;
					const int32 TestY = Y + dy;
					const int32 Index = TestY * InvW + TestX;

					if (Occupied.IsValidIndex(Index) && Occupied[Index])
					{
						bCanPlace = false;
					}
				}
			}

			if (bCanPlace)
			{
				NewEntry.CellX = X;
				NewEntry.CellY = Y;

				InventoryItems.Add(NewEntry);

				UE_LOG(LogTemp, Log, TEXT("AddItemToInventory: %s placed at (%d, %d)"),
					*ItemRow.InternalName.ToString(), X, Y);

				// обновляем виджет инвентаря, если открыт
				if (InventoryLayoutWidgetInstance)
				{
					UE_LOG(LogTemp, Warning, TEXT("AddItemToInventory: DistributeItems, count=%d"), InventoryItems.Num());
					InventoryLayoutWidgetInstance->DistributeItems(InventoryItems);
					InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
				}

				return true;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("AddItemToInventory: no free space for %s"),
		*ItemRow.InternalName.ToString());

	return false;
}

void AARESMMOCharacter::PickupWorldItem(class AWorldItemActor* WorldItem)
{
	if (!WorldItem)
	{
		return;
	}

	const int32 WorldItemID = WorldItem->ItemID;
	const int32 StackCount  = FMath::Max(1, WorldItem->StackCount);

	if (WorldItemID <= 0)
	{
		return;
	}

	const FItemBaseRow* Row = ItemDB::GetItemByID(WorldItemID);
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("PickupWorldItem: ItemID %d not found in DB"), WorldItemID);
		return;
	}

	const bool bAdded = AddItemToInventory(*Row, StackCount);
	if (bAdded)
	{
		WorldItem->Destroy();
	}
}

void AARESMMOCharacter::RefreshInventoryUI()
{
	if (InventoryLayoutWidgetInstance)
	{
		InventoryLayoutWidgetInstance->DistributeItems(InventoryItems);
		InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
	}
}

bool AARESMMOCharacter::MoveInventoryItem(FName InternalName, int32 FromCellX, int32 FromCellY, int32 ToCellX,
                                          int32 ToCellY)
{
	if (InventoryWidthCells <= 0 || InventoryHeightCells <= 0)
		return false;

	const int32 Index = InventoryItems.IndexOfByPredicate(
		[&](const FInventoryItemEntry& E)
		{
			return E.CellX == FromCellX &&
				   E.CellY == FromCellY &&
				   E.ItemRow.InternalName == InternalName;
		});

	if (Index == INDEX_NONE)
		return false;

	FInventoryItemEntry& Entry = InventoryItems[Index];

	const int32 MaxX = FMath::Max(0, InventoryWidthCells  - Entry.SizeInCells.Width);
	const int32 MaxY = FMath::Max(0, InventoryHeightCells - Entry.SizeInCells.Height);

	const int32 ClampedX = FMath::Clamp(ToCellX, 0, MaxX);
	const int32 ClampedY = FMath::Clamp(ToCellY, 0, MaxY);

	if (ClampedX == Entry.CellX && ClampedY == Entry.CellY)
		return true;

	if (!CanPlaceInInventory(InventoryItems, InventoryWidthCells, InventoryHeightCells, Entry.SizeInCells, ClampedX, ClampedY, Index))
		return false;

	Entry.CellX = ClampedX;
	Entry.CellY = ClampedY;

	if (InventoryLayoutWidgetInstance)
	{
		InventoryLayoutWidgetInstance->DistributeItems(InventoryItems);
		InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
	}

	return true;
}

bool AARESMMOCharacter::EquipInventoryItemToSlot(FName InternalName, int32 FromCellX, int32 FromCellY,
	EEquipmentSlotType TargetSlot)
{
	const int32 Index = InventoryItems.IndexOfByPredicate(
		[&](const FInventoryItemEntry& E)
		{
			return E.CellX == FromCellX &&
				   E.CellY == FromCellY &&
				   E.ItemRow.InternalName == InternalName;
		});

	if (Index == INDEX_NONE)
		return false;

	const FInventoryItemEntry DraggedEntry = InventoryItems[Index];
	const FItemBaseRow& ItemRow = DraggedEntry.ItemRow;

	// Определяем “родной” слот предмета
	EEquipmentSlotType BaseSlot = ItemRow.EquipmentSlot;
	const bool bIsHeroPart = (ItemRow.StoreCategory == EStoreCategory::storecat_HeroParts);

	if (BaseSlot == EEquipmentSlotType::None)
	{
		if (bIsHeroPart)
		{
			BaseSlot = GetHeroPartSlot(ItemRow.HeroPartType);
		}
		else
		{
			BaseSlot = GetEquipmentSlotForCategory(ItemRow.StoreCategory);
		}
	}

	// Проверка совместимости со SlotType, на который дропнули
	bool bCompatible = (BaseSlot == TargetSlot);

	// Weapon1/Weapon2 — взаимозаменяемы
	if (!bCompatible && IsWeaponSlotType(BaseSlot) && IsWeaponSlotType(TargetSlot))
		bCompatible = true;

	// Device1/Device2 — взаимозаменяемы
	if (!bCompatible && IsDeviceSlotType(BaseSlot) && IsDeviceSlotType(TargetSlot))
		bCompatible = true;

	if (!bCompatible || TargetSlot == EEquipmentSlotType::None)
		return false;

	// Если слот занят — swap (старый эквип → на место dragged в инвентарь, если помещается)
	if (const FItemBaseRow* OldEquipped = EquipmentSlots.Find(TargetSlot))
	{
		FInventoryItemEntry SwapCandidate;
		SwapCandidate.ItemRow     = *OldEquipped;
		SwapCandidate.CellX       = FromCellX;
		SwapCandidate.CellY       = FromCellY;
		SwapCandidate.Quantity    = OldEquipped->bUseStack ? 1 : 1;

		SwapCandidate.SizeInCells = OldEquipped->GridSize;
		if (SwapCandidate.SizeInCells.Width <= 0 || SwapCandidate.SizeInCells.Height <= 0)
		{
			SwapCandidate.SizeInCells = UItemSizeRules::GetDefaultSize(OldEquipped->ItemClass);
		}

		if (!CanPlaceInInventory(InventoryItems, InventoryWidthCells, InventoryHeightCells,
			SwapCandidate.SizeInCells, SwapCandidate.CellX, SwapCandidate.CellY, Index))
		{
			return false;
		}

		// swap в инвентаре (не меняем длину массива)
		InventoryItems[Index] = SwapCandidate;
	}
	else
	{
		// слот пуст — удаляем предмет из инвентаря
		InventoryItems.RemoveAt(Index);
	}

	// Записываем в экипировку ИМЕННО в TargetSlot (без ResolveAutoSlot)
	EquipmentSlots.FindOrAdd(TargetSlot) = ItemRow;

	// Применяем визуал для HeroParts/Gear
	const bool bIsGear =
		(ItemRow.StoreCategory == EStoreCategory::storecat_Armor  ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_Helmet ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_Mask   ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_Backpack);

	if (bIsHeroPart)
	{
		EquipHeroPart(ItemRow);
	}
	else if (bIsGear)
	{
		EquipEquipment(ItemRow);
	}
	else
	{
		// оружие/девайсы — стойку пересчитаем ниже
	}
	
	if (TargetSlot == EEquipmentSlotType::EquipmentSlotPistol ||
		TargetSlot == EEquipmentSlotType::EquipmentSlotWeapon1 ||
		TargetSlot == EEquipmentSlotType::EquipmentSlotWeapon2)
	{
		EquipWeaponActorToSlot(ItemRow, TargetSlot);
	}

	// Пересчёт стойки (важно при Weapon/Pistol/Knife)
	if (IsWeaponAffectingSlot(TargetSlot))
	{
		RecalculateWeaponStateFromEquipment();
	}

	// UI
	if (InventoryLayoutWidgetInstance)
	{
		InventoryLayoutWidgetInstance->DistributeItems(InventoryItems);
		InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
	}

	return true;
}

bool AARESMMOCharacter::UnequipSlotToInventoryAt(EEquipmentSlotType SlotType, int32 ToCellX, int32 ToCellY)
{
	FItemBaseRow* Found = EquipmentSlots.Find(SlotType);
	if (!Found)
		return false;

	const FItemBaseRow RemovedItem = *Found;

	FInventoryItemEntry Candidate;
	Candidate.ItemRow  = RemovedItem;
	Candidate.Quantity = RemovedItem.bUseStack ? 1 : 1;

	// FIX: используем GridSize из ItemRow если он задан, иначе default по классу
	Candidate.SizeInCells = RemovedItem.GridSize;
	if (Candidate.SizeInCells.Width <= 0 || Candidate.SizeInCells.Height <= 0)
	{
		Candidate.SizeInCells = UItemSizeRules::GetDefaultSize(RemovedItem.ItemClass);
	}

	const int32 MaxX = FMath::Max(0, InventoryWidthCells  - Candidate.SizeInCells.Width);
	const int32 MaxY = FMath::Max(0, InventoryHeightCells - Candidate.SizeInCells.Height);

	Candidate.CellX = FMath::Clamp(ToCellX, 0, MaxX);
	Candidate.CellY = FMath::Clamp(ToCellY, 0, MaxY);

	if (!CanPlaceInInventory(InventoryItems, InventoryWidthCells, InventoryHeightCells,
		Candidate.SizeInCells, Candidate.CellX, Candidate.CellY, INDEX_NONE))
	{
		return false;
	}

	// ===== Снимаем визуал (как в UnequipSlot), но оружие теперь через WeaponActor =====
	switch (SlotType)
	{
	case EEquipmentSlotType::EquipmentSlotHelmet:
		if (Mesh_Helmet) Mesh_Helmet->SetSkeletalMesh(nullptr);
		break;

	case EEquipmentSlotType::EquipmentSlotArmor:
		if (Mesh_Armor) Mesh_Armor->SetSkeletalMesh(nullptr);
		break;

	case EEquipmentSlotType::EquipmentSlotMask:
		if (Mesh_Mask) Mesh_Mask->SetSkeletalMesh(nullptr);
		break;

	case EEquipmentSlotType::EquipmentSlotBackpack:
		if (Mesh_Backpack) Mesh_Backpack->SetSkeletalMesh(nullptr);
		break;

	case EEquipmentSlotType::EquipmentSlotHead:
		if (Mesh_Head) Mesh_Head->SetSkeletalMesh(DefaultHeadMesh);
		break;

	case EEquipmentSlotType::EquipmentSlotBody:
		if (Mesh_Body) Mesh_Body->SetSkeletalMesh(DefaultBodyMesh);
		break;

	case EEquipmentSlotType::EquipmentSlotLegs:
		if (Mesh_Legs) Mesh_Legs->SetSkeletalMesh(DefaultLegsMesh);
		break;
	
	case EEquipmentSlotType::EquipmentSlotPistol:
	{
		DestroyWeaponActorInSlot(EEquipmentSlotType::EquipmentSlotPistol);
	}
	
	case EEquipmentSlotType::EquipmentSlotWeapon1:
	case EEquipmentSlotType::EquipmentSlotWeapon2:
	{
		DestroyWeaponActorInSlot(SlotType);
	}

	default:
		break;
	}

	// Удаляем из экипировки и кладём в инвентарь
	EquipmentSlots.Remove(SlotType);
	InventoryItems.Add(Candidate);

	// Пересчёт стойки
	if (IsWeaponAffectingSlot(SlotType))
	{
		RecalculateWeaponStateFromEquipment();
	}

	// UI
	if (InventoryLayoutWidgetInstance)
	{
		InventoryLayoutWidgetInstance->DistributeItems(InventoryItems);
		InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
	}

	return true;
}

bool AARESMMOCharacter::ContextMenu_Equip(FName InternalName, int32 FromCellX, int32 FromCellY)
{
	const int32 Index = FindInventoryIndexForAction(InventoryItems, InternalName, FromCellX, FromCellY);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	const FItemBaseRow ItemRow = InventoryItems[Index].ItemRow;

	EquipItem(ItemRow);
	InventoryItems.RemoveAt(Index);

	RefreshInventoryUI();
	return true;
}

bool AARESMMOCharacter::ContextMenu_Attach(FName InternalName, int32 FromCellX, int32 FromCellY)
{
	const int32 Index = FindInventoryIndexForAction(InventoryItems, InternalName, FromCellX, FromCellY);
	if (Index == INDEX_NONE)
		return false;

	const FItemBaseRow ItemRow = InventoryItems[Index].ItemRow;

	// только WeaponATTM
	if (ItemRow.StoreCategory != EStoreCategory::storecat_WeaponATTM)
	{
		UE_LOG(LogTemp, Warning, TEXT("ContextMenu_Attach: not a weapon attachment: %s"), *ItemRow.InternalName.ToString());
		return false;
	}

	AWeaponBase* TargetWeapon = GetBestWeaponForAttachment();
	if (!TargetWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("ContextMenu_Attach: no weapon equipped"));
		return false;
	}

	if (!TargetWeapon->AttachItem(ItemRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("ContextMenu_Attach: attach failed: %s"), *ItemRow.InternalName.ToString());
		return false;
	}

	// убираем аттач из инвентаря
	InventoryItems.RemoveAt(Index);
	RefreshInventoryUI();
	return true;
}

bool AARESMMOCharacter::ContextMenu_Use(FName InternalName, int32 FromCellX, int32 FromCellY)
{
	const int32 Index = FindInventoryIndexForAction(InventoryItems, InternalName, FromCellX, FromCellY);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	FInventoryItemEntry& Entry = InventoryItems[Index];
	const FItemBaseRow& ItemRow = Entry.ItemRow;

	// PlaceItem — пока заглушка (BuildingSystem позже)
	if (ItemRow.StoreCategory == EStoreCategory::storecat_PlaceItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("ContextMenu_Use: TODO PlaceItem/BuildingSystem. Item=%s"), *ItemRow.InternalName.ToString());
		return true;
	}

	UseItem(ItemRow);

	// Consume 1 если это стак
	if (ItemRow.bUseStack)
	{
		Entry.Quantity = FMath::Max(0, Entry.Quantity - 1);
		if (Entry.Quantity <= 0)
		{
			InventoryItems.RemoveAt(Index);
		}
	}
	else
	{
		InventoryItems.RemoveAt(Index);
	}

	RefreshInventoryUI();
	return true;
}

bool AARESMMOCharacter::ContextMenu_Study(FName InternalName, int32 FromCellX, int32 FromCellY)
{
	const int32 Index = FindInventoryIndexForAction(InventoryItems, InternalName, FromCellX, FromCellY);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	const FItemBaseRow& ItemRow = InventoryItems[Index].ItemRow;

	// Заглушка под “изучение рецепта”
	UE_LOG(LogTemp, Warning, TEXT("ContextMenu_Study: TODO learn recipe. Item=%s"), *ItemRow.InternalName.ToString());
	return true;
}

bool AARESMMOCharacter::ContextMenu_Drop(FName InternalName, int32 FromCellX, int32 FromCellY)
{
	const int32 Index = FindInventoryIndexForAction(InventoryItems, InternalName, FromCellX, FromCellY);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	const FInventoryItemEntry Dropped = InventoryItems[Index];
	InventoryItems.RemoveAt(Index);

	// Спавним WorldItemActor перед игроком
	if (UWorld* W = GetWorld())
	{
		const FVector SpawnLoc = GetActorLocation() + GetActorForwardVector() * 120.f + FVector(0.f, 0.f, 30.f);
		const FRotator SpawnRot = FRotator::ZeroRotator;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AWorldItemActor* WorldItem = W->SpawnActor<AWorldItemActor>(AWorldItemActor::StaticClass(), SpawnLoc, SpawnRot, Params);
		if (WorldItem)
		{
			const int32 StackCount = Dropped.ItemRow.bUseStack ? FMath::Max(1, Dropped.Quantity) : 1;
			WorldItem->InitFromItemID(Dropped.ItemRow.ItemID, StackCount);
		}
	}

	RefreshInventoryUI();
	return true;
}

bool AARESMMOCharacter::ContextMenu_ChargeItem(FName InternalName, int32 FromCellX, int32 FromCellY)
{
	const int32 TargetIndex = FindInventoryIndexForAction(InventoryItems, InternalName, FromCellX, FromCellY);
	if (TargetIndex == INDEX_NONE)
	{
		return false;
	}

	FInventoryItemEntry& Target = InventoryItems[TargetIndex];

	// предмет должен юзать заряд
	if (!Target.ItemRow.bUseCharge || Target.ItemRow.MaxCharge <= 0)
	{
		return false;
	}

	const int32 BatteryIndex = FindBySubCategory(InventoryItems, EStoreSubCategory::Item_Battery);
	if (BatteryIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("ContextMenu_ChargeItem: no batteries"));
		return false;
	}

	// Снимаем 1 батарейку
	FInventoryItemEntry& Battery = InventoryItems[BatteryIndex];
	if (Battery.ItemRow.bUseStack)
	{
		Battery.Quantity = FMath::Max(0, Battery.Quantity - 1);
		if (Battery.Quantity <= 0)
		{
			InventoryItems.RemoveAt(BatteryIndex);
		}
	}
	else
	{
		InventoryItems.RemoveAt(BatteryIndex);
	}

	Target.ItemRow.DefaultCharge = Target.ItemRow.MaxCharge;

	RefreshInventoryUI();
	return true;
}

bool AARESMMOCharacter::ContextMenu_ChargeMagazine(FName InternalName, int32 FromCellX, int32 FromCellY)
{
	const int32 TargetIndex = FindInventoryIndexForAction(InventoryItems, InternalName, FromCellX, FromCellY);
	if (TargetIndex == INDEX_NONE)
	{
		return false;
	}

	FInventoryItemEntry& Mag = InventoryItems[TargetIndex];

	if (Mag.ItemRow.StoreSubCategory != EStoreSubCategory::WeaponATTM_Magazine)
	{
		return false;
	}

	// Магазин теперь использует AMMO
	if (!Mag.ItemRow.bUseAmmo || Mag.ItemRow.MaxAmmo <= 0)
	{
		return false;
	}

	const int32 Need = FMath::Max(0, Mag.ItemRow.MaxAmmo - Mag.ItemRow.CurrAmmo);
	if (Need <= 0)
	{
		return true; // уже полный
	}

	const EStoreSubCategory NeedAmmoType = Mag.ItemRow.AcceptedAmmoSubCategory;
	int32 AmmoIndex = INDEX_NONE;

	if (NeedAmmoType != EStoreSubCategory::None)
	{
		AmmoIndex = FindAmmoIndexBySubCategory(InventoryItems, NeedAmmoType);
	}
	else
	{
		AmmoIndex = FindAnyAmmoIndex(InventoryItems); // fallback
	}

	if (AmmoIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("ContextMenu_ChargeMagazine: no ammo for type=%d"), (int32)NeedAmmoType);
		return false;
	}

	FInventoryItemEntry& Ammo = InventoryItems[AmmoIndex];

	const int32 Have = Ammo.ItemRow.bUseStack ? FMath::Max(0, Ammo.Quantity) : 1;
	const int32 Take = FMath::Min(Need, Have);

	Mag.ItemRow.CurrAmmo += Take;

	// consume ammo
	if (Ammo.ItemRow.bUseStack)
	{
		Ammo.Quantity = FMath::Max(0, Ammo.Quantity - Take);
		if (Ammo.Quantity <= 0)
		{
			InventoryItems.RemoveAt(AmmoIndex);
		}
	}
	else
	{
		InventoryItems.RemoveAt(AmmoIndex);
	}

	RefreshInventoryUI();
	return true;
}

bool AARESMMOCharacter::ContextMenu_Repair(FName InternalName, int32 FromCellX, int32 FromCellY)
{
	const int32 TargetIndex = FindInventoryIndexForAction(InventoryItems, InternalName, FromCellX, FromCellY);
	if (TargetIndex == INDEX_NONE)
	{
		return false;
	}

	FInventoryItemEntry& Target = InventoryItems[TargetIndex];

	if (!Target.ItemRow.bUseDurability || Target.ItemRow.MaxDurability <= 0.f)
	{
		return false;
	}

	const int32 KitIndex = FindBySubCategory(InventoryItems, EStoreSubCategory::Item_RapairKit);
	if (KitIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("ContextMenu_Repair: no repair kit"));
		return false;
	}

	FInventoryItemEntry& Kit = InventoryItems[KitIndex];

	// 1 use = -1 charge (если charge не настроен, считаем что 4 по умолчанию)
	if (!Kit.ItemRow.bUseCharge)
	{
		Kit.ItemRow.bUseCharge = true;
		Kit.ItemRow.MaxCharge = 4;
		if (Kit.ItemRow.DefaultCharge <= 0) Kit.ItemRow.DefaultCharge = 4;
	}

	Kit.ItemRow.DefaultCharge = FMath::Max(0, Kit.ItemRow.DefaultCharge - 1);
	if (Kit.ItemRow.DefaultCharge <= 0)
	{
		InventoryItems.RemoveAt(KitIndex);
	}

	Target.ItemRow.DefaultDurability = Target.ItemRow.MaxDurability;

	RefreshInventoryUI();
	return true;
}
