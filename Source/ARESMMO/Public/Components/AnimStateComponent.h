#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animations/AnimType.h"
#include "Animations/AnimClips.h"
#include "AnimStateComponent.generated.h"

class ACharacter;
class UCharacterMovementComponent;
class USkeletalMeshComponent;
class UAnimSequence;

UCLASS(ClassGroup=(ARES), meta=(BlueprintSpawnableComponent))
class UAnimStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnimStateComponent();

	// === ТЕКУЩИЕ СОСТОЯНИЯ ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim")
	EWeaponState WeaponState = EWeaponState::Unarmed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim")
	EAnimState AnimState = EAnimState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim")
	EMoveDirection MoveDirection = EMoveDirection::None;

	// Нормализованный input (Y=Forward, X=Right)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Anim")
	FVector2D LastMoveInput = FVector2D::ZeroVector;

	// ===== Конфиг анимаций =====
	// Можно заполнять в C++ (через LoadObject) или в Default'ах компонента
	UPROPERTY(EditAnywhere, Category="Anim")
	TMap<EWeaponState, FWeaponAnimationSet> AnimConfig;

	// === Публичные апдейты ===

	// Смена стойки по оружию
	void SetWeaponState(EWeaponState NewState);

	// Сюда прокидываем чистый input из Move()
	void UpdateMoveInput(const FVector2D& MoveInput);

	// Сюда прокидываем флаги движения (Tick или при изменениях)
	void UpdateMovementFlags(bool bInSprint, bool bInCrouch, bool bInAir, float Speed);

protected:
	virtual void BeginPlay() override;

private:
	TWeakObjectPtr<ACharacter> OwnerCharacter;
	TWeakObjectPtr<UCharacterMovementComponent> MoveComp;
	TWeakObjectPtr<USkeletalMeshComponent> Mesh;

	void RecalculateAnimState(bool bInSprint, bool bInCrouch, bool bInAir, float Speed);
	void RecalculateDirection();

	// === ЛОГИКА ВЫБОРА И ПРОИГРЫША АНИМАЦИИ ===
	void UpdateAnimation();
	UAnimSequence* SelectCurrentClip() const;
};