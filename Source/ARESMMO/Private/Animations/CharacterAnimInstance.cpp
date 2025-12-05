#include "Animations/CharacterAnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "ARESMMO/ARESMMOCharacter.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"

void UCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	CharacterRef = Cast<AARESMMOCharacter>(TryGetPawnOwner());
	if (CharacterRef)
	{
		CharacterMovement = CharacterRef->GetCharacterMovement();
	}
}

void UCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!CharacterRef)
	{
		CharacterRef = Cast<AARESMMOCharacter>(TryGetPawnOwner());
		if (CharacterRef)
		{
			CharacterMovement = CharacterRef->GetCharacterMovement();
		}
	}

	if (!CharacterRef || !CharacterMovement)
	{
		return;
	}
	
	UpdateAcceleration();
	
	bIsCrouching = CharacterMovement->IsCrouching();
	bIsSprint    = CharacterRef->bIsSprinting;
	bIsFalling   = CharacterMovement->IsFalling();
	bIsJump      = bIsFalling;
	bIsAim = CharacterRef->bIsAiming;
	
	const FRotator AimRot   = CharacterRef->GetBaseAimRotation();
	const FRotator ActorRot = CharacterRef->GetActorRotation();
	const FRotator DeltaRot = (AimRot - ActorRot);
	Pitch = DeltaRot.Pitch;
	
	UpdateDirectionAndOrientation(DeltaSeconds);
	UpdateTurnInPlace(DeltaSeconds);
	UpdateWallDetection();
	UpdateRunningIntoWall();
	
	AnimationState = CharacterRef->GetWeaponState();

	bIsPistolEquip     = (AnimationState == EWeaponState::Pistol);
	bIsRifleEquip      = (AnimationState == EWeaponState::Rifle);
	bIsShotgunEquip    = (AnimationState == EWeaponState::Shotgun);
	bIsMeleeEquip      = (AnimationState == EWeaponState::Melee);
	bIsGrenadeEquip    = (AnimationState == EWeaponState::Grenade);
	bIsPlaceItemEquip  = (AnimationState == EWeaponState::PlaceItem);
	bIsUsableItemEquip = (AnimationState == EWeaponState::UsableItem);
	bIsWeaponEquip     = (AnimationState != EWeaponState::Unarmed);
	
	if (AController* Controller = CharacterRef->GetController())
	{
		const FRotator ControlRot = Controller->GetControlRotation();
		const float DeltaYaw = (ControlRot - WorldRotation).Yaw;
		YawOffset      = DeltaYaw;
	}
}

void UCharacterAnimInstance::UpdateRunningIntoWall()
{
	if (!CharacterMovement || !CharacterRef)
	{
		bIsWall = false;
		return;
	}

	if (bRunningIntoWall)
	{
		bIsWall = true;
	}
	else
	{
		bIsWall = false;
	}
}

void UCharacterAnimInstance::UpdateWallDetection()
{
	bRunningIntoWall = false;

	const float AccelLen = LocalAcceleration2D.Size2D();
	const float VelLen   = LocalVelocity2D.Size2D();
	const bool bAccelBigEnough = (AccelLen > 0.1f);
	const bool bVelocityLow    = (VelLen < 200.f);
	const FVector AccelN = LocalAcceleration2D.GetSafeNormal2D(0.0001f);
	const FVector VelN   = LocalVelocity2D.GetSafeNormal2D(0.0001f);
	const float Dot = FVector::DotProduct(AccelN, VelN);
	const bool bDotInRange = (Dot >= -0.6f && Dot <= 0.6f);

	bRunningIntoWall = bAccelBigEnough && bVelocityLow && bDotInRange;
}

void UCharacterAnimInstance::UpdateAcceleration()
{
	if (!CharacterRef || !CharacterMovement)
	{
		bHasAcceleration    = false;
		WorldAcceleration2D = FVector::ZeroVector;
		LocalAcceleration2D = FVector::ZeroVector;
		WorldVelocity       = FVector::ZeroVector;
		WorldVelocity2D     = FVector::ZeroVector;
		LocalVelocity2D     = FVector::ZeroVector;
		WorldRotation       = FRotator::ZeroRotator;
		return;
	}
	
	WorldRotation = CharacterRef->GetActorRotation();
	const FVector CurrentAccel = CharacterMovement->GetCurrentAcceleration();
	WorldAcceleration2D = FVector(CurrentAccel.X, CurrentAccel.Y, 0.f);
	LocalAcceleration2D = WorldRotation.UnrotateVector(WorldAcceleration2D);
	WorldVelocity   = CharacterRef->GetVelocity();
	WorldVelocity2D = FVector(WorldVelocity.X, WorldVelocity.Y, 0.f);
	LocalVelocity2D = WorldRotation.UnrotateVector(WorldVelocity2D);
	
	const float AccelLenSq2D =
		FMath::Square(LocalAcceleration2D.X) +
		FMath::Square(LocalAcceleration2D.Y);

	const bool bNearlyZero = FMath::IsNearlyEqual(AccelLenSq2D, 0.f, 0.000001f);
	bHasAcceleration = !bNearlyZero;
	
	GroundSpeed = WorldVelocity2D.Size();
	bShouldMove = (GroundSpeed > 3.f) && bHasAcceleration;
}

void UCharacterAnimInstance::UpdateDirectionAndOrientation(float DeltaSeconds)
{
	const FVector Velocity2D = FVector(WorldVelocity.X, WorldVelocity.Y, 0.f);

	// базовый ротатор — куда смотрит камера / контроллер
	const FRotator AimRot = CharacterRef->GetBaseAimRotation();

	Direction = UKismetAnimationLibrary::CalculateDirection(
		Velocity2D,
		AimRot
	);
	
	E_MovementInput = EMoveDirection::None;

	if (GroundSpeed > 3.f)
	{
		// Вперёд только узкий сектор [-45; 45]
		if (Direction > -45.f && Direction < 45.f)
		{
			E_MovementInput = EMoveDirection::Forward;
		}
		// Право [45;135]
		else if (Direction >= 45.f && Direction <= 135.f)
		{
			E_MovementInput = EMoveDirection::Right;
		}
		// Лево [-135;-45]
		else if (Direction <= -45.f && Direction >= -135.f)
		{
			E_MovementInput = EMoveDirection::Left;
		}
		else
		{
			E_MovementInput = EMoveDirection::Backward;
		}
	}

	F_OrientationAngle = (E_MovementInput == EMoveDirection::Forward)  ? 1.f : 0.f;
	R_OrientationAngle = (E_MovementInput == EMoveDirection::Right)    ? 1.f : 0.f;
	B_OrientationAngle = (E_MovementInput == EMoveDirection::Backward) ? 1.f : 0.f;
	L_OrientationAngle = (E_MovementInput == EMoveDirection::Left)     ? 1.f : 0.f;
	
	DirectionAngle = FMath::FInterpTo(DirectionAngle, Direction, DeltaSeconds, 10.f);
}

void UCharacterAnimInstance::UpdateTurnInPlace(float DeltaSeconds)
{
	if (!CharacterRef || !CharacterMovement)
	{
		RootYawOffset   = 0.f;
		AbsRootYawOffset = 0.f;
		YawExcess       = 0.f;
		return;
	}

	if (CharacterRef->IsFirstPerson())
	{
		RootYawOffset   = 0.f;
		return;
	}
	
	if (bShouldMove || bIsFalling)
	{
		RootYawOffset   = FMath::FInterpTo(RootYawOffset, 0.f, DeltaSeconds, 10.f);
		AbsRootYawOffset = FMath::Abs(RootYawOffset);
		YawExcess       = 0.f;
		return;
	}
	
	AController* Controller = CharacterRef->GetController();
	if (!Controller)
	{
		RootYawOffset   = FMath::FInterpTo(RootYawOffset, 0.f, DeltaSeconds, 10.f);
		AbsRootYawOffset = FMath::Abs(RootYawOffset);
		YawExcess       = 0.f;
		return;
	}

	const FRotator ControlRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FRotator ActorRot(0.f, CharacterRef->GetActorRotation().Yaw, 0.f);
	
	const float TargetOffset =
		UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, ActorRot).Yaw;
	
	RootYawOffset = FMath::FInterpTo(RootYawOffset, TargetOffset, DeltaSeconds, 8.f);
	
	RootYawOffset = FMath::Clamp(RootYawOffset, -90.f, 90.f);

	AbsRootYawOffset = FMath::Abs(RootYawOffset);
	YawExcess        = 0.f;
}

void UCharacterAnimInstance::ForceResetTurnInPlace()
{
	RootYawOffset = 0.f;
	AbsRootYawOffset = 0.f;
	YawExcess = 0.f;
	LastDistanceCurve = 0.f;
	DistanceCurve = 0.f;
	MaxTurnAngle = 0.f;
}