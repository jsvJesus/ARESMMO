#include "ARESMMOCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "UI/Game/GameHUDWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PlayerStatsComponent.h"
#include "Components/CapsuleComponent.h"
#include "Items/ItemData.h"
#include "Items/ItemTypes.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

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

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

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

	// ===== Player StatsComponent =====
	Stats = CreateDefaultSubobject<UPlayerStatsComponent>(TEXT("PlayerStats"));
}

void AARESMMOCharacter::BeginPlay()
{
	Super::BeginPlay();

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
}

void AARESMMOCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// синхроним флаг crouch для AnimBP
		bIsCrouchedAnim = Move->IsCrouching();
	}

	// Уменьшаем стамину при спринте
	if (bIsSprinting && Stats)
	{
		Stats->ConsumeStamina(SprintStaminaCostPerSecond * DeltaSeconds);

		// Если стамина кончилась — вырубаем спринт
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
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
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

/* Direction */
void AARESMMOCharacter::GetOrientationAngles(float Direction, float& F_Orientation_Angle, float& R_Orientation_Angle,
	float& L_Orientation_Angle, float& B_Orientation_Angle) const
{
	const float Forward  = Direction - 0.0f;
	const float Right    = Direction - 90.0f;
	const float Left     = Direction - (-90.0f);
	const float Backward = Direction - 180.0f;

	// Нормализуем градусы в диапазон -180..180, чтобы не плодить 720, 1080 и т.п.
	F_Orientation_Angle = FMath::UnwindDegrees(Forward);
	R_Orientation_Angle = FMath::UnwindDegrees(Right);
	L_Orientation_Angle = FMath::UnwindDegrees(Left);
	B_Orientation_Angle = FMath::UnwindDegrees(Backward);
}

void AARESMMOCharacter::SetWeaponAnimStateFromItem(const FItemBaseRow& ItemRow)
{
	EWeaponAnimState NewState = EWeaponAnimState::Unarmed;

	switch (ItemRow.StoreCategory)
	{
	case EStoreCategory::storecat_ASR:
	case EStoreCategory::storecat_SNP:
	case EStoreCategory::storecat_SMG:
		NewState = EWeaponAnimState::Rifle;
		break;
	case EStoreCategory::storecat_HG:
		NewState = EWeaponAnimState::Pistol;
		break;
	case EStoreCategory::storecat_SHTG:
	case EStoreCategory::storecat_MG:
		NewState = EWeaponAnimState::Shotgun;
		break;
	case EStoreCategory::storecat_MELEE:
		NewState = EWeaponAnimState::Melee;
		break;
	case EStoreCategory::storecat_Grenade:
		NewState = EWeaponAnimState::Grenade;
		break;
	case EStoreCategory::storecat_PlaceItem:
		NewState = EWeaponAnimState::PlaceItem;
		break;
	default:
		NewState = EWeaponAnimState::Unarmed;
		break;
	}

	SetWeaponAnimState(NewState); // теперь обновит и WeaponState, и булки
}

void AARESMMOCharacter::SetWeaponAnimStateFromItem(const FItemBaseRow* ItemRow)
{
	if (!ItemRow)
	{
		SetWeaponAnimState(EWeaponAnimState::Unarmed);
		return;
	}

	SetWeaponAnimStateFromItem(*ItemRow);
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
		// Части тела (модульный персонаж)
	case EStoreCategory::storecat_HeroParts:
		EquipHeroPart(ItemRow);
		break;

		// Экипировка, которая вешается на персонажа
	case EStoreCategory::storecat_Armor:
	case EStoreCategory::storecat_Helmet:
	case EStoreCategory::storecat_Mask:
	case EStoreCategory::storecat_Backpack:
		EquipEquipment(ItemRow);
		break;

		// Оружие, гранаты, placeable — меняют AnimState
	case EStoreCategory::storecat_ASR:
	case EStoreCategory::storecat_SNP:
	case EStoreCategory::storecat_SHTG:
	case EStoreCategory::storecat_HG:
	case EStoreCategory::storecat_MG:
	case EStoreCategory::storecat_SMG:
	case EStoreCategory::storecat_MELEE:
	case EStoreCategory::storecat_Grenade:
	case EStoreCategory::storecat_PlaceItem:
		SetWeaponAnimStateFromItem(ItemRow);
		break;

		// Остальные категории пока не экипируем (Medicine/Food/Water/UsableItem и т.д.)
	default:
		UE_LOG(LogTemp, Log, TEXT("EquipItem: Item %s (category %d) has no equip logic yet"),
			   *ItemRow.InternalName.ToString(),
			   static_cast<int32>(ItemRow.StoreCategory));
		break;
	}
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

void AARESMMOCharacter::SetWeaponState(EWeaponAnimState NewState)
{
	// основное состояние
	WeaponState = NewState;

	// флаги под разные стойки
	bHasRifle     = (NewState == EWeaponAnimState::Rifle);
	bHasPistol    = (NewState == EWeaponAnimState::Pistol);
	bHasShotgun   = (NewState == EWeaponAnimState::Shotgun);
	bHasMelee     = (NewState == EWeaponAnimState::Melee);
	bHasGrenade   = (NewState == EWeaponAnimState::Grenade);
	bHasPlaceItem = (NewState == EWeaponAnimState::PlaceItem);
}

void AARESMMOCharacter::SetWeaponAnimState(EWeaponAnimState NewState)
{
	CurrentWeaponAnimState = NewState;

	// обновляем WeaponState + булки
	SetWeaponState(NewState);
}

void AARESMMOCharacter::UpdateMovementStateFromDirection(float Direction)
{
	// Нормализуем угол в -180..180, чтобы не ловить 360/720 и т.п.
	const float Normalized = FMath::UnwindDegrees(Direction);

	EMovementState NewState = EMovementState::None;

	// 1) Forward: -70 .. 70
	if (Normalized >= -70.0f && Normalized <= 70.0f)
	{
		NewState = EMovementState::Forward;
	}
	// 2) Right: 70 .. 110
	else if (Normalized > 70.0f && Normalized <= 110.0f)
	{
		NewState = EMovementState::Right;
	}
	// 3) Left: -110 .. -70
	else if (Normalized >= -110.0f && Normalized < -70.0f)
	{
		NewState = EMovementState::Left;
	}
	// 4) Остальное — Backward
	else
	{
		NewState = EMovementState::Backward;
	}

	MovementState = NewState;
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
		Move->MaxWalkSpeed = bIsCrouchedAnim ? CrouchSpeed : WalkSpeed;
	}
}