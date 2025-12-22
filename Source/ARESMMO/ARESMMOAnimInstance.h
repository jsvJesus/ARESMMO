#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	TWeakObjectPtr<AARESMMOCharacter> CachedCharacter;
};