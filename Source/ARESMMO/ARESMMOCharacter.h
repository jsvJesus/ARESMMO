#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/GameCode/Weapon/WeaponConfig.h"
#include "Logging/LogMacros.h"
#include "ARESMMOCharacter.generated.h"

class UHeroConfig;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USkeletalMeshComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	None      UMETA(DisplayName="None"),
	Forward   UMETA(DisplayName="Forward"),
	Backward  UMETA(DisplayName="Backward"),
	Left      UMETA(DisplayName="Left"),
	Right     UMETA(DisplayName="Right"),
};

UCLASS(config=Game)
class AARESMMOCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Camera Action */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Movement Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction; // Jump
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction; // Walk

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction; // Sprinting

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction; // Crouching
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction; // Look

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InventoryAction; // Inventory (Button "i")
	
public:
	AARESMMOCharacter();
	
protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprinting(const FInputActionValue& Value); // Sprinting
	void Crouching(const FInputActionValue& Value); // Crouching
	/** Обработчик нажатия на InventoryAction ("i") */
	void OnInventoryPressed(const FInputActionValue& Value);

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	
	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** применить HeroConfig к модульным мешам (можно вызывать из блюпринта) */
	UFUNCTION(BlueprintCallable, Category="ARES|Hero")
	void ApplyHeroConfig();

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** ARESMMO: конфиг героя с массивами Head/Body/Legs/Hands */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ARES|Hero")
	TObjectPtr<UHeroConfig> HeroConfig;

	/** ARESMMO: модульные части героя (отдельные SkeletalMeshComponent'ы) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Hero")
	USkeletalMeshComponent* HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Hero")
	USkeletalMeshComponent* BodyMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Hero")
	USkeletalMeshComponent* LegsMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Hero")
	USkeletalMeshComponent* HandsMesh;

	/** ARESMMO: Movement State */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Movement")
	EMovementState MovementState = EMovementState::None;

	// ===== Movement speeds =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement")
	float CrouchSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement")
	float SprintSpeed = 600.0f;

	/** True, когда персонаж присел */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Movement")
	bool bIsCrouchedAnim = false;

	/** True, когда персонаж бежит (спринт) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Movement")
	bool bIsSprinting = false;

	/* Direction */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Movement")
	void GetOrientationAngles(
		float Direction,
		float& F_Orientation_Angle,
		float& R_Orientation_Angle,
		float& L_Orientation_Angle,
		float& B_Orientation_Angle
	) const;

	/**
	 * Обновляет MovementState на основе Direction,
	 *  -70..70    → Forward
	 *  70..110    → Right
	 *  -110..-70  → Left
	 *  иначе      → Backward
	 */
	UFUNCTION(BlueprintCallable, Category="ARES|Movement")
	void UpdateMovementStateFromDirection(float Direction);

	// Вызов из инпута / BP
	UFUNCTION(BlueprintCallable, Category="ARES|Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category="ARES|Movement")
	void StopSprint();

	/** ARESMMO: выбранные индексы вариаций в HeroConfig */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Hero", meta=(ClampMin="0"))
	int32 HeadIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Hero", meta=(ClampMin="0"))
	int32 BodyIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Hero", meta=(ClampMin="0"))
	int32 LegsIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Hero", meta=(ClampMin="0"))
	int32 HandsIndex = 0;
	
	/** Оружие по умолчанию, которое можно выбрать на персонаже в редакторе */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ARES|Weapon")
	TObjectPtr<UWeaponConfig> DefaultWeaponConfig = nullptr;

	/** Текущее оружие в руках (рантайм) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Weapon")
	TObjectPtr<UWeaponConfig> CurrentWeaponConfig = nullptr;

	/** Флаг, что персонаж полностью без оружия */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Weapon")
	bool bIsUnarmed = true;

	/** Аним-тип текущего оружия (Assault / Pistol / SMG / RPG / Melee / и т.д.) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Weapon")
	EWeaponAnimType CurrentWeaponAnimType = EWeaponAnimType::Assault;

	/** true, если персонаж совсем без оружия */
	UFUNCTION(BlueprintPure, Category="ARES|Weapon")
	bool HasUnarmed() const;

	UFUNCTION(BlueprintPure, Category="ARES|Weapon")
	bool HasPistol() const;

	UFUNCTION(BlueprintPure, Category="ARES|Weapon")
	bool HasRifle() const;

	UFUNCTION(BlueprintPure, Category="ARES|Weapon")
	bool HasShotgun() const;

	/** Эквипнуть WeaponConfig (можно звать из BP/инвентаря) */
	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	void EquipWeaponConfig(UWeaponConfig* NewWeaponConfig);

	/** Убрать оружие из рук (перейти в Unarmed) */
	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	void UnequipWeaponConfig();
};

