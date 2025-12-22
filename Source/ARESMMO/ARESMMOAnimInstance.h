#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animations/AnimType.h"
#include "ARESMMOAnimInstance.generated.h"

class AARESMMOCharacter;

UCLASS(Blueprintable)
class ARESMMO_API UARESMMOAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// Эти переменные читает AnimGraph (TwoBoneIK / TransformBone)
	UPROPERTY(BlueprintReadOnly, Category="ARES|IK")
	float HandsIKWeight = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|IK")
	FTransform HandLTransform = FTransform::Identity;

	UPROPERTY(BlueprintReadOnly, Category="ARES|IK")
	FTransform HandRTransform = FTransform::Identity;

	// ===== Movement (READ ONLY from Character) =====
	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement")
	float GroundSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement")
	bool ShouldMove = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement")
	bool IsFalling = false;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement")
	float DirectionAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Movement")
	float TurnRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Client")
	EWeaponState Client_WeaponState = EWeaponState::Unarmed;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Client")
	EMoveDirection Client_MovementState = EMoveDirection::None;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation")
	float F_OrientationAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation")
	float R_OrientationAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation")
	float B_OrientationAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Orientation")
	float L_OrientationAngle = 0.0f;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	TWeakObjectPtr<AARESMMOCharacter> CachedCharacter;
};