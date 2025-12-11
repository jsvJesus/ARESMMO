#include "ARESMMOCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "UI/Game/GameHUDWidget.h"
#include "UI/Game/Inventory/InventoryWidget.h"
#include "UI/Game/Inventory/InventoryLayoutWidget.h"
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
	Mesh_Head = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh_Head")); // Head
	Mesh_Head->SetupAttachment(GetMesh());
	Mesh_Head->SetLeaderPoseComponent(GetMesh());
	
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
	FPSCamera->SetupAttachment(GetMesh(), TEXT("head")); // имя сокета головы своё
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

				// Прокидываем в виджет нашего Pawn, чтобы он нашёл UPlayerStatsComponent
				GameHUDWidgetInstance->InitFromPawn(this);
			}
		}
	}

	if (InventoryRenderTarget)
	{
		InventoryCapture->TextureTarget = InventoryRenderTarget;

		// Очищаем и добавляем только нужные компоненты
		InventoryCapture->ShowOnlyComponents.Empty();

		InventoryCapture->ShowOnlyComponents.Add(GetMesh());
		InventoryCapture->ShowOnlyComponents.Add(Mesh_Head);
		InventoryCapture->ShowOnlyComponents.Add(Mesh_Body);
		InventoryCapture->ShowOnlyComponents.Add(Mesh_Legs);
		InventoryCapture->ShowOnlyComponents.Add(Mesh_Armor);
		InventoryCapture->ShowOnlyComponents.Add(Mesh_Helmet);
		InventoryCapture->ShowOnlyComponents.Add(Mesh_Mask);
		InventoryCapture->ShowOnlyComponents.Add(Mesh_Backpack);
	}

	if (InventoryCapture && InventoryRenderTarget)
	{
		InventoryCapture->TextureTarget     = InventoryRenderTarget;
		InventoryCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

		// Полный аналог узла Show Only Actor Components (Self)
		InventoryCapture->ShowOnlyActorComponents(this, true); // true = брать компоненты детей
	}
}

void AARESMMOCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// --------- Стамина от спринта ---------
	if (bIsSprinting && Stats)
	{
		Stats->ConsumeStamina(SprintStaminaCostPerSecond * DeltaSeconds);

		if (Stats->Base.Stamina <= 0.f)
		{
			StopSprint();
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
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
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

void AARESMMOCharacter::EquipHeroPart(const FItemBaseRow& ItemRow)
{
	if (ItemRow.HeroPartType == EHeroPartType::None || !ItemRow.HeroPartMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipHeroPart: Invalid item or mesh."));
		return;
	}

	switch (ItemRow.HeroPartType)
	{
	case EHeroPartType::Head:
		Mesh_Head->SetSkeletalMesh(ItemRow.HeroPartMesh);
		break;

	case EHeroPartType::Body:
		Mesh_Body->SetSkeletalMesh(ItemRow.HeroPartMesh);
		break;

	case EHeroPartType::Legs:
		Mesh_Legs->SetSkeletalMesh(ItemRow.HeroPartMesh);
		break;
	default: ;
	}

	UE_LOG(LogTemp, Log, TEXT("Equipped mesh for part: %d"), (int)ItemRow.HeroPartType);
}

void AARESMMOCharacter::EquipEquipment(const FItemBaseRow& ItemRow)
{
	if (!ItemRow.HeroPartMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipEquipment: Invalid mesh for item %s"),
			*ItemRow.InternalName.ToString());
		return;
	}

	switch (ItemRow.StoreCategory)
	{
	case EStoreCategory::storecat_Armor:
		Mesh_Armor->SetSkeletalMesh(ItemRow.HeroPartMesh);
		break;

	case EStoreCategory::storecat_Helmet:
		Mesh_Helmet->SetSkeletalMesh(ItemRow.HeroPartMesh);
		break;

	case EStoreCategory::storecat_Mask:
		Mesh_Mask->SetSkeletalMesh(ItemRow.HeroPartMesh);
		break;

	case EStoreCategory::storecat_Backpack:
		Mesh_Backpack->SetSkeletalMesh(ItemRow.HeroPartMesh);
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("EquipEquipment: Item %s is not wearable"),
			*ItemRow.InternalName.ToString());
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Equipped equipment: %s"),
		   *ItemRow.InternalName.ToString());
}

void AARESMMOCharacter::EquipItem(const FItemBaseRow& ItemRow)
{
	switch (ItemRow.StoreCategory)
	{
	case EStoreCategory::storecat_HeroParts:
		EquipHeroPart(ItemRow);
		return;

	case EStoreCategory::storecat_Armor:
	case EStoreCategory::storecat_Helmet:
	case EStoreCategory::storecat_Mask:
	case EStoreCategory::storecat_Backpack:
		EquipEquipment(ItemRow);
		return;

	default:
		break;
	}

	// Всё остальное: оружие / спец-предметы
	const EWeaponState NewState = GetWeaponStateForCategory(ItemRow.StoreCategory);

	if (NewState != EWeaponState::Unarmed)
	{
		SetWeaponState(NewState);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("EquipItem: Item %s (category %d) does not change WeaponState"),
			   *ItemRow.InternalName.ToString(),
			   static_cast<int32>(ItemRow.StoreCategory));
	}

	// Определяем базовый слот по DataTable или по категории
	EEquipmentSlotType Slot = ItemRow.EquipmentSlot;
	if (Slot == EEquipmentSlotType::None)
	{
		Slot = GetEquipmentSlotForCategory(ItemRow.StoreCategory);
	}

	// Автологика Weapon1/Weapon2, Device1/Device2
	Slot = ResolveAutoSlot(Slot, EquipmentSlots);

	// Если слот валидный — кладём предмет туда
	if (Slot != EEquipmentSlotType::None)
	{
		EquipmentSlots.FindOrAdd(Slot) = ItemRow;

		// Обновляем UI
		if (InventoryLayoutWidgetInstance)
		{
			InventoryLayoutWidgetInstance->SetEquipment(EquipmentSlots);
		}
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

	// Сбрасываем визуальные меши, если нужно
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
		if (Mesh_Head)  Mesh_Head->SetSkeletalMesh(nullptr);
		break;
	case EEquipmentSlotType::EquipmentSlotBody:
		if (Mesh_Body)  Mesh_Body->SetSkeletalMesh(nullptr);
		break;
	case EEquipmentSlotType::EquipmentSlotLegs:
		if (Mesh_Legs)  Mesh_Legs->SetSkeletalMesh(nullptr);
		break;

	default:
		break;
	}

	// TODO: пересчитать WeaponState в зависимости от того, что осталось в Weapon1/Weapon2

	// Пытаемся вернуть предмет в инвентарь
	const bool bAdded = AddItemToInventory(RemovedItem, 1);
	if (!bAdded)
	{
		// нет места — откатываем (чтоб не терять предмет)
		EquipmentSlots.Add(SlotType, RemovedItem);
		EquipItem(RemovedItem); // вернём визуал/weapon state
		UE_LOG(LogTemp, Warning, TEXT("UnequipSlot: no space in inventory for %s, reverting"),
			*RemovedItem.InternalName.ToString());
		return false;
	}

	// Обновляем панель экипировки в UI
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
	switch (Category)
	{
	case EStoreCategory::storecat_ASR:
	case EStoreCategory::storecat_SNP:
	case EStoreCategory::storecat_MG:
		return EWeaponState::Rifle;

	case EStoreCategory::storecat_SHTG:
		return EWeaponState::Shotgun;

	case EStoreCategory::storecat_HG:
	case EStoreCategory::storecat_SMG:
		return EWeaponState::Pistol;

	case EStoreCategory::storecat_MELEE:
		return EWeaponState::Melee;

	case EStoreCategory::storecat_Grenade:
		return EWeaponState::Grenade;

	case EStoreCategory::storecat_PlaceItem:
		return EWeaponState::PlaceItem;

	case EStoreCategory::storecat_UsableItem:
		return EWeaponState::UsableItem;

	default:
		return EWeaponState::Unarmed;
	}
}

void AARESMMOCharacter::StartSprint()
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		if (Move->IsCrouching())
		{
			return;
		}

		// Проверка статов
		if (Stats)
		{
			// если нет стамины — не бежать
			if (Stats->Base.Stamina <= 0.f)
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
	bUseControllerRotationYaw = false;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// OrientRotationToMovement всегда OFF
		Move->bOrientRotationToMovement = true;
	}

	// --- Скрываем шмотку на голове в FPS ---
	//if (Mesh_Head)   Mesh_Head->SetVisibility(false, true);
	if (Mesh_Helmet) Mesh_Helmet->SetVisibility(false, true);
	if (Mesh_Mask)   Mesh_Mask->SetVisibility(false, true);
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
	//if (Mesh_Head)   Mesh_Head->SetVisibility(false, true);
	if (Mesh_Helmet) Mesh_Helmet->SetVisibility(true, true);
	if (Mesh_Mask)   Mesh_Mask->SetVisibility(true, true);
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
	NewEntry.SizeInCells = UItemSizeRules::GetDefaultSize(ItemRow.ItemClass);

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

void AARESMMOCharacter::PickupWorldItem(AWorldItemActor* WorldItem)
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