#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animations/AnimType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CharacterAnimInstance.generated.h"

class AARESMMOCharacter;

UCLASS()
class ARESMMO_API UCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// ============== Components ==============
	UPROPERTY(BlueprintReadOnly, Category="ARES|Components", meta=(AllowPrivateAccess="true"))
	AARESMMOCharacter* CharacterRef = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Components", meta=(AllowPrivateAccess="true"))
	UCharacterMovementComponent* CharacterMovement = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Components", meta=(AllowPrivateAccess="true"))
	EWeaponState AnimationState = EWeaponState::Unarmed;

	// ============== World Acceleration ==============
	UPROPERTY(BlueprintReadOnly, Category="ARES|World Acceleration", meta=(AllowPrivateAccess="true"))
	FVector WorldAcceleration2D = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="ARES|World Acceleration", meta=(AllowPrivateAccess="true"))
	FRotator WorldRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category="ARES|World Acceleration", meta=(AllowPrivateAccess="true"))
	FVector LocalAcceleration2D = FVector::ZeroVector;

	// ============== World Velocity ==============
	UPROPERTY(BlueprintReadOnly, Category="ARES|World Velocity", meta=(AllowPrivateAccess="true"))
	FVector WorldVelocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="ARES|World Velocity", meta=(AllowPrivateAccess="true"))
	FVector WorldVelocity2D = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="ARES|World Velocity", meta=(AllowPrivateAccess="true"))
	FVector LocalVelocity2D = FVector::ZeroVector;

	// ============== Acceleration flags ==============
	UPROPERTY(BlueprintReadOnly, Category="ARES|Acceleration", meta=(AllowPrivateAccess="true"))
	bool bHasAcceleration = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Acceleration", meta=(AllowPrivateAccess="true"))
	bool bRunningIntoWall = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Acceleration", meta=(AllowPrivateAccess="true"))
	bool bIsWall = false;

	// ============== Alpha ==============
	UPROPERTY(BlueprintReadWrite, Category="ARES|Alpha", meta=(AllowPrivateAccess="true"))
	float Alpha = 1.0f;

	UPROPERTY(BlueprintReadWrite, Category="ARES|Alpha", meta=(AllowPrivateAccess="true"))
	float UpperBodyAlpha = 1.0f;

	// ============== Turn in Place ==============
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float YawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float Pitch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float RootYawOffset = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	FRotator MovingRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	FRotator LastMovingRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float DistanceCurve = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float MaxTurnAngle = 90.f; // new var for TurnInPlace

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	FName DistanceToPivot = TEXT("DistanceToPivot");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	FName Turning = TEXT("Turning");

	UPROPERTY(BlueprintReadOnly, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float LastDistanceCurve = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float DeltaDistanceCurve = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float AbsRootYawOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float YawExcess = 0.0f;

	// ============== Weapon flags ==============
	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsAim = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsPistolEquip = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsRifleEquip = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsShotgunEquip = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsMeleeEquip = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsGrenadeEquip = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsPlaceItemEquip = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsUsableItemEquip = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon", meta=(AllowPrivateAccess="true"))
	bool bIsWeaponEquip = false;

	// ============== Orientation Angle ==============
	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation Angle", meta=(AllowPrivateAccess="true"))
	float F_OrientationAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation Angle", meta=(AllowPrivateAccess="true"))
	float R_OrientationAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation Angle", meta=(AllowPrivateAccess="true"))
	float B_OrientationAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation Angle", meta=(AllowPrivateAccess="true"))
	float L_OrientationAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation Angle", meta=(AllowPrivateAccess="true"))
	float DirectionAngle = 0.0f;

	// ============== Movement ==============
	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	float GroundSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	bool bShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	bool bIsFalling = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	float Direction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	EMoveDirection E_MovementInput = EMoveDirection::None;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	bool bIsCrouching = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	bool bIsSprint = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	ELandState LandState = ELandState::None;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement", meta=(AllowPrivateAccess="true"))
	bool bIsJump = false;
	
	void UpdateWallDetection();     // считает bRunningIntoWall
	void UpdateRunningIntoWall();   // реагирует на флаг: IsWall + MaxWalkSpeed
	void UpdateAcceleration();
	void UpdateDirectionAndOrientation(float DeltaSeconds);
	void UpdateTurnInPlace(float DeltaSeconds);
	
public:
	UFUNCTION(BlueprintCallable)
	void ForceResetTurnInPlace();
};