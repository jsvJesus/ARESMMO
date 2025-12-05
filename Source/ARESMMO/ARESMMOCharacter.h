#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Items/ItemData.h"
#include "Animations/AnimType.h"
#include "ARESMMOCharacter.generated.h"

class UPlayerStatsComponent;
class UGameHUDWidget;
class UInventoryLayoutWidget;
class UAresCharacterAnimInstance;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USkeletalMeshComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class USceneComponent;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AARESMMOCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Camera TPS Action */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* TPSCamera;

	/** Camera FPS Action */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FPSCamera;

	// ===== Inventory Preview =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|InventoryPreview", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* InventoryCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|InventoryPreview", meta=(AllowPrivateAccess="true"))
	USceneComponent* InventoryPreviewPivot;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleViewAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InventoryAction;

	// ПКМ — чисто Zoom камеры, не AimOffset
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;
	
public:
	AARESMMOCharacter();

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprinting(const FInputActionValue& Value);
	void Crouching(const FInputActionValue& Value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Camera")
	bool bIsFirstPerson = false;

	UFUNCTION(BlueprintCallable, Category="ARES|Camera")
	void ToggleCameraMode();

	void SwitchToFPS();
	void SwitchToTPS();
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Inventory =====
	UFUNCTION(BlueprintCallable, Category="ARES|UI")
	void ToggleInventory();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|UI")
	TSubclassOf<UInventoryLayoutWidget> InventoryLayoutWidgetClass;

	UPROPERTY()
	UInventoryLayoutWidget* InventoryLayoutWidgetInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|UI")
	bool bIsInventoryOpen = false;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetTPSCamera() const { return TPSCamera; }
	FORCEINLINE class UCameraComponent* GetFPSCamera() const { return FPSCamera; }

	UFUNCTION(BlueprintPure, Category="ARES|Camera")
	bool IsFirstPerson() const { return bIsFirstPerson; }

	// Поворот персонажа для превью
	UFUNCTION(BlueprintCallable, Category="ARES|InventoryPreview")
	void AddInventoryPreviewYaw(float DeltaYaw);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|InventoryPreview")
	UTextureRenderTarget2D* InventoryRenderTarget;

	// ===== Movement speeds =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement")
	float CrouchSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement")
	float SprintSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Movement")
	bool bIsSprinting = false;

	// Вызов из инпута / BP
	UFUNCTION(BlueprintCallable, Category="ARES|Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category="ARES|Movement")
	void StopSprint();

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

	// ===== PLAYER STATS =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Stats")
	UPlayerStatsComponent* Stats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement")
	float SprintStaminaCostPerSecond = 15.0f;

	// ===== UI / Game HUD =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|UI")
	TSubclassOf<UGameHUDWidget> GameHUDWidgetClass;

	UPROPERTY()
	UGameHUDWidget* GameHUDWidgetInstance = nullptr;

	// HeroParts
	UFUNCTION(BlueprintCallable, Category="ARES|HeroParts")
	void EquipHeroPart(const FItemBaseRow& ItemRow);
	
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void EquipEquipment(const FItemBaseRow& ItemRow);

	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void EquipItem(const FItemBaseRow& ItemRow);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void UseItem(const FItemBaseRow& ItemRow);

	// ===== Zoom (ПКМ, только TPS, без AimOffset) =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Camera")
	bool bIsAiming = false; // тут только зум TPS

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Camera|Aim")
	float TPSDefaultArmLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Camera|Aim")
	float TPSDefaultFOV = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Camera|Aim")
	float TPSAimArmLength = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Camera|Aim")
	float TPSAimFOV = 70.0f;

	UFUNCTION(BlueprintCallable, Category="ARES|Camera")
	void StartAim();  // Zoom

	UFUNCTION(BlueprintCallable, Category="ARES|Camera")
	void StopAim();   // Zoom off

	// ===== Weapon State (стойка оружия) =====
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ARES|Anim")
	EWeaponState WeaponState = EWeaponState::Unarmed; // Default стойка Unarmed

	UFUNCTION(BlueprintCallable, Category="ARES|Anim")
	void SetWeaponState(EWeaponState NewState);

	UFUNCTION(BlueprintPure, Category="ARES|Anim")
	EWeaponState GetWeaponState() const { return WeaponState; }

	EWeaponState GetWeaponStateForCategory(EStoreCategory Category);
};
