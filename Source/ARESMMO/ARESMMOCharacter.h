#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Items/ItemData.h"
#include "ARESMMOCharacter.generated.h"

class UAresCharacterAnimInstance;
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

UENUM(BlueprintType)
enum class EWeaponAnimState : uint8
{
	Unarmed    UMETA(DisplayName="Unarmed"),
	Rifle      UMETA(DisplayName="Rifle"),
	Pistol     UMETA(DisplayName="Pistol"),
	Shotgun    UMETA(DisplayName="Shotgun"),
	Melee      UMETA(DisplayName="Melee"),
	Grenade    UMETA(DisplayName="Grenade"),
	PlaceItem  UMETA(DisplayName="Place Item")
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
	
public:
	AARESMMOCharacter();
	
protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprinting(const FInputActionValue& Value); // Sprinting
	void Crouching(const FInputActionValue& Value); // Crouching
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	
	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

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

	/** Текущее оружейное состояние анимации */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim")
	EWeaponAnimState CurrentWeaponAnimState = EWeaponAnimState::Unarmed;

	/** Текущее состояние оружейной стойки */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Anim")
	EWeaponAnimState WeaponState = EWeaponAnimState::Unarmed;

	/** Флаги под разные слои / state-машины в AnimBP */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Anim") bool bHasRifle     = false;
	UPROPERTY(BlueprintReadOnly, Category="ARES|Anim") bool bHasPistol    = false;
	UPROPERTY(BlueprintReadOnly, Category="ARES|Anim") bool bHasShotgun   = false;
	UPROPERTY(BlueprintReadOnly, Category="ARES|Anim") bool bHasMelee     = false;
	UPROPERTY(BlueprintReadOnly, Category="ARES|Anim") bool bHasGrenade   = false;
	UPROPERTY(BlueprintReadOnly, Category="ARES|Anim") bool bHasPlaceItem = false;

	/** Установить AnimState по строке предмета (из DataTable) */
	UFUNCTION(BlueprintCallable, Category="ARES|Anim")
	void SetWeaponAnimStateFromItem(const FItemBaseRow& ItemRow);

	/** Принудительно установить AnimState (Unarmed, Rifle и т.п.) */
	UFUNCTION(BlueprintCallable, Category="ARES|Anim")
	void SetWeaponAnimState(EWeaponAnimState NewState);

	/** Вызывать только из C++ (персонаж / инвентарь) */
	UFUNCTION(BlueprintCallable, Category="ARES|Anim")
	void SetWeaponState(EWeaponAnimState NewState);
	
	UFUNCTION(BlueprintCallable, Category="ARES|Movement")
	void UpdateMovementStateFromDirection(float Direction);

	// Вызов из инпута / BP
	UFUNCTION(BlueprintCallable, Category="ARES|Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category="ARES|Movement")
	void StopSprint();

	void SetWeaponAnimStateFromItem(const FItemBaseRow* ItemRow);

	// ===== Modular Character Parts =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|HeroParts")
	USkeletalMeshComponent* Mesh_Head;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|HeroParts")
	USkeletalMeshComponent* Mesh_Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|HeroParts")
	USkeletalMeshComponent* Mesh_Legs;

	// ===== Equipment Visual Meshes =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|EquipmentMesh")
	USkeletalMeshComponent* Mesh_Armor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|EquipmentMesh")
	USkeletalMeshComponent* Mesh_Helmet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|EquipmentMesh")
	USkeletalMeshComponent* Mesh_Mask;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|EquipmentMesh")
	USkeletalMeshComponent* Mesh_Backpack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Equipment")
	TMap<EEquipmentSlotType, FItemBaseRow> EquipmentSlots;

	// Функция экипировки части тела
	UFUNCTION(BlueprintCallable, Category="ARES|HeroParts")
	void EquipHeroPart(const FItemBaseRow& ItemRow);
	
	// Функция экипировки предмета экипировки (броня/шлем/маска/рюкзак)
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void EquipEquipment(const FItemBaseRow& ItemRow);

	// Универсальная функция экипировки любого предмета по категории
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void EquipItem(const FItemBaseRow& ItemRow);
};

