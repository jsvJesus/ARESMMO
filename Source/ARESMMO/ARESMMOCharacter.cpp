#include "ARESMMOCharacter.h"

#include "ARESMMOPlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/GameCode/Hero/HeroConfig.h"
#include "Kismet/KismetMathLibrary.h"

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

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	//GetCharacterMovement()->JumpZVelocity = 700.f;
	//GetCharacterMovement()->AirControl = 0.35f;
	//GetCharacterMovement()->MaxWalkSpeed = 500.f;
	//GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	//GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	//GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// создаём модульные SkeletalMeshComponent'ы, которые повторяют позу основного Mesh
	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetMesh());
	HeadMesh->SetLeaderPoseComponent(GetMesh());

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetMesh());
	BodyMesh->SetLeaderPoseComponent(GetMesh());

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh());
	LegsMesh->SetLeaderPoseComponent(GetMesh());

	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh"));
	HandsMesh->SetupAttachment(GetMesh());
	HandsMesh->SetLeaderPoseComponent(GetMesh());
}

void AARESMMOCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Применяем HeroConfig сразу при спавне персонажа
	ApplyHeroConfig();

	// Если на персе в редакторе выбрано оружие — сразу эквипаем
	if (DefaultWeaponConfig)
	{
		EquipWeaponConfig(DefaultWeaponConfig);
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

		// Inventory (кнопка "I")
		if (InventoryAction)
		{
			EnhancedInputComponent->BindAction(
				InventoryAction,
				ETriggerEvent::Started,
				this,
				&AARESMMOCharacter::OnInventoryPressed
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

void AARESMMOCharacter::ApplyHeroConfig()
{
	if (!HeroConfig)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("ApplyHeroConfig: HeroConfig is null on %s"), *GetName());
		return;
	}
	
	// if (HeroConfig)
	// {
	//     HeadMesh->SetSkeletalMesh(HeroConfig->GetHeadMesh(HeadIndex));
	//     LegsMesh->SetSkeletalMesh(HeroConfig->GetLegMesh(LegsIndex));
	//     BodyMesh->SetSkeletalMesh(HeroConfig->GetBodyMesh(BodyIndex, /*bIsFPS=*/false));
	//     HandsMesh->SetSkeletalMesh(HeroConfig->GetHandsMesh(HandsIndex));
	// }

	if (HeadMesh)
	{
		HeadMesh->SetSkeletalMesh(HeroConfig->GetHeadMesh(HeadIndex));
	}

	if (LegsMesh)
	{
		LegsMesh->SetSkeletalMesh(HeroConfig->GetLegMesh(LegsIndex));
	}

	if (BodyMesh)
	{
		BodyMesh->SetSkeletalMesh(HeroConfig->GetBodyMesh(BodyIndex, /*bIsFPS=*/false));
	}

	if (HandsMesh)
	{
		HandsMesh->SetSkeletalMesh(HeroConfig->GetHandsMesh(HandsIndex));
	}
}

void AARESMMOCharacter::EquipWeaponConfig(UWeaponConfig* NewWeaponConfig)
{
	CurrentWeaponConfig = NewWeaponConfig;

	if (!NewWeaponConfig)
	{
		// Полностью без оружия
		bIsUnarmed            = true;
		CurrentWeaponAnimType = EWeaponAnimType::Unarmed;
		return;
	}

	bIsUnarmed            = false;
	CurrentWeaponAnimType = NewWeaponConfig->AnimType;
}

void AARESMMOCharacter::UnequipWeaponConfig()
{
	CurrentWeaponConfig   = nullptr;
	bIsUnarmed            = true;
	CurrentWeaponAnimType = EWeaponAnimType::Unarmed;
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

// ======== Геттеры для AnimBP (BP_Manny) ========
bool AARESMMOCharacter::HasUnarmed() const
{
	return bIsUnarmed;
}

bool AARESMMOCharacter::HasPistol() const
{
	return !bIsUnarmed && CurrentWeaponAnimType == EWeaponAnimType::Pistol;
}

bool AARESMMOCharacter::HasRifle() const
{
	if (bIsUnarmed)
	{
		return false;
	}

	// Rifle-стойка: ASR / SNP / SMG / RPG -> у тебя всё это в одном AnimType::Assault (+ RPG отдельно)
	switch (CurrentWeaponAnimType)
	{
	case EWeaponAnimType::Assault: // ASR + SNP + SMG
	case EWeaponAnimType::RPG:     // если потом добавишь ракетницы
		return true;

	default:
		return false;
	}
}

bool AARESMMOCharacter::HasShotgun() const
{
	// SHTG аним-тип: SHTG + MG
	return !bIsUnarmed && CurrentWeaponAnimType == EWeaponAnimType::SHTG;
}

void AARESMMOCharacter::OnInventoryPressed(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("OnInventoryPressed called, Value = %s"), *Value.ToString());

	if (AARESMMOPlayerController* PC = Cast<AARESMMOPlayerController>(Controller))
	{
		PC->ToggleInventory();
	}
}