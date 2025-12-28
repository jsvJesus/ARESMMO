#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Items/ItemData.h"
#include "Animations/AnimType.h"
#include "ARESMMOCharacter.generated.h"

struct FInventoryItemEntry;
class UPlayerStatsComponent;
class UGameHUDWidget;
class UInventoryLayoutWidget;
class UInventoryWidget;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USkeletalMeshComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class USceneComponent;
struct FInputActionValue;
class AWorldItemActor;
class AWeaponBase;
class AInventoryPreviewCaptureActor;

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

	/** FPS camera local offset relative to Mesh (not to Head socket) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|FPS", meta = (AllowPrivateAccess = "true"))
	FVector FPSCameraLocalOffset = FVector(0.f, 0.f, 160.f);

	/** FPS camera local rotation relative to Mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera|FPS", meta = (AllowPrivateAccess = "true"))
	FRotator FPSCameraLocalRotation = FRotator::ZeroRotator;
	
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

	// ===== Inventory Preview =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|InventoryPreview", meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* InventoryCapture;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|InventoryPreview", meta=(AllowPrivateAccess="true"))
	USceneComponent* InventoryPreviewPivot = nullptr;

	FORCEINLINE USceneComponent* GetInventoryPreviewPivot() const { return InventoryPreviewPivot; }

	FORCEINLINE class USkeletalMeshComponent* GetFPSHandsMesh() const { return Mesh_FPS_Hand; }

	UFUNCTION(BlueprintCallable, Category="ARES|InventoryPreview")
	void UpdateInventoryPreviewShowOnly();

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ===== Anim data (calculated on Character) =====
	void UpdateAnimMovementData(float DeltaSeconds);

	// ===== Inventory =====
	UFUNCTION(BlueprintCallable, Category="ARES|UI")
	void ToggleInventory();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|UI")
	TSubclassOf<UInventoryLayoutWidget> InventoryLayoutWidgetClass;

	UPROPERTY()
	UInventoryLayoutWidget* InventoryLayoutWidgetInstance = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|UI")
	bool bIsInventoryOpen = false;

	// Чувствительность мыши (для самой камеры)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Camera")
	float MouseYawSensitivity = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Camera")
	float MousePitchSensitivity = 1.0f;

	// Нормализация TurnRate (подбирается под мышь)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Camera")
	float TurnRateScale = 0.10f;

	UFUNCTION(BlueprintCallable, Category="ARES|Anim")
	void RecalculateWeaponStateFromEquipment();

	// ===== Weapon Actor Helpers =====
	AWeaponBase* GetWeaponActorInSlot(EEquipmentSlotType SlotType) const;
	void SetWeaponActorInSlot(EEquipmentSlotType SlotType, AWeaponBase* Weapon);

	bool EquipWeaponActorToSlot(const FItemBaseRow& ItemRow, EEquipmentSlotType SlotType);
	void DestroyWeaponActorInSlot(EEquipmentSlotType SlotType);

	// Какое оружие считать "активным" для Attach (пока приоритет Weapon1->Weapon2->Pistol)
	AWeaponBase* GetBestWeaponForAttachment() const;
	
	void SetSelectedWeaponInternal(AWeaponBase* NewWeapon);

	UPROPERTY()
	TObjectPtr<AInventoryPreviewCaptureActor> InventoryPreviewActor = nullptr;

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
	USkeletalMeshComponent* Mesh_Hair;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|HeroParts")
	USkeletalMeshComponent* Mesh_Beard;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|HeroParts")
	USkeletalMeshComponent* Mesh_Head;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|HeroParts")
	USkeletalMeshComponent* Mesh_Hand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|HeroParts")
	USkeletalMeshComponent* Mesh_Body;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|HeroParts")
	USkeletalMeshComponent* Mesh_Legs;

	/** First-person hands mesh (owner only) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category ="Camera|FPS")
	USkeletalMeshComponent* Mesh_FPS_Hand;

	// Дефолтные меши для восстановления при снятии вещей
	UPROPERTY()
	USkeletalMesh* DefaultHairMesh = nullptr;
        
	UPROPERTY()
	USkeletalMesh* DefaultBeardMesh = nullptr;
	
	UPROPERTY()
    USkeletalMesh* DefaultHeadMesh = nullptr;

	UPROPERTY()
	USkeletalMesh* DefaultHandMesh = nullptr;

	UPROPERTY()
	USkeletalMesh* DefaultFPSHandMesh = nullptr;
    
    UPROPERTY()
    USkeletalMesh* DefaultBodyMesh = nullptr;
    
    UPROPERTY()
    USkeletalMesh* DefaultLegsMesh = nullptr;

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

	// ===== Weapon Actors (NEW) =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|WeaponActors")
	TObjectPtr<AWeaponBase> WeaponActor_Weapon1 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|WeaponActors")
	TObjectPtr<AWeaponBase> WeaponActor_Weapon2 = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|WeaponActors")
	TObjectPtr<AWeaponBase> WeaponActor_Pistol = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|WeaponActors")
	TObjectPtr<AWeaponBase> SelectedWeapon = nullptr;

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	bool DetachWeaponATTMToInventory(EStoreSubCategory SubCategory);

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	void SelectWeaponSlot(EEquipmentSlotType SlotType);

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	AWeaponBase* GetSelectedWeapon() const { return SelectedWeapon; }

	// ===== PLAYER STATS =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Stats")
	UPlayerStatsComponent* Stats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement")
	float SprintStaminaCostPerSecond = 15.0f;

	// ===== Sprint / Exhaustion =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement|Stamina")
	float MinStaminaPercentToSprint = 15.0f; // 15% = 15.0 (т.к. стамина 0..100)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Movement|Stamina")
	float ExhaustedMaxWalkSpeed = 0.0f; // 0 = персонаж стоит, пока не восстановится

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Movement|Stamina")
	bool bIsExhausted = false;

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

	// ===== Equipment =====
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	bool UnequipSlot(EEquipmentSlotType SlotType);

	// ===== Inventory / Fast Equip =====
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	bool EquipItemFromInventory(const FItemBaseRow& ItemRow);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void UseItem(const FItemBaseRow& ItemRow);

	/** Простое хранилище предметов персонажа (сеточный инвентарь) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Inventory")
	TArray<FInventoryItemEntry> InventoryItems;

	/** Размер инвентаря в клетках (должен совпадать с UInventoryWidget) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory")
	int32 InventoryWidthCells = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory")
	int32 InventoryHeightCells = 11;

	/** Добавить предмет в инвентарь (по строке DataTable) */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	bool AddItemToInventory(const FItemBaseRow& ItemRow, int32 StackCount);

	/** Подобрать актор предмета в мире */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void PickupWorldItem(class AWorldItemActor* WorldItem);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void RefreshInventoryUI();

	// ===== Drag&Drop (Inventory <-> Equipment) =====
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|DragDrop")
	bool MoveInventoryItem(FName InternalName, int32 FromCellX, int32 FromCellY, int32 ToCellX, int32 ToCellY);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|DragDrop")
	bool EquipInventoryItemToSlot(FName InternalName, int32 FromCellX, int32 FromCellY, EEquipmentSlotType TargetSlot);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|DragDrop")
	bool UnequipSlotToInventoryAt(EEquipmentSlotType SlotType, int32 ToCellX, int32 ToCellY);

	// ===== Context Menu Actions =====
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	bool ContextMenu_Equip(FName InternalName, int32 FromCellX, int32 FromCellY);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	bool ContextMenu_Attach(FName InternalName, int32 FromCellX, int32 FromCellY);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	bool ContextMenu_Use(FName InternalName, int32 FromCellX, int32 FromCellY);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	bool ContextMenu_Study(FName InternalName, int32 FromCellX, int32 FromCellY);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	bool ContextMenu_Drop(FName InternalName, int32 FromCellX, int32 FromCellY);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	bool ContextMenu_ChargeItem(FName InternalName, int32 FromCellX, int32 FromCellY);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	bool ContextMenu_ChargeMagazine(FName InternalName, int32 FromCellX, int32 FromCellY);

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	bool ContextMenu_Repair(FName InternalName, int32 FromCellX, int32 FromCellY);

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

	// ============== World Acceleration ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|World Acceleration", meta=(AllowPrivateAccess="true"))
	FVector WorldAcceleration2D = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|World Acceleration", meta=(AllowPrivateAccess="true"))
	FRotator WorldRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|World Acceleration", meta=(AllowPrivateAccess="true"))
	FVector LocalAcceleration2D = FVector::ZeroVector;

	// ============== Velocity ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Velocity", meta=(AllowPrivateAccess="true"))
	FVector WorldVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Velocity", meta=(AllowPrivateAccess="true"))
	FVector WorldVelocity2D = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Velocity", meta=(AllowPrivateAccess="true"))
	FVector LocalVelocity2D = FVector::ZeroVector;

	// ============== Acceleration flags ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Acceleration", meta=(AllowPrivateAccess="true"))
	bool bHasAcceleration = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Acceleration", meta=(AllowPrivateAccess="true"))
	bool bRunningIntoWall = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Acceleration", meta=(AllowPrivateAccess="true"))
	bool bIsWall = false;

	// ============== Alpha ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Alpha", meta=(AllowPrivateAccess="true"))
	float Alpha = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Alpha", meta=(AllowPrivateAccess="true"))
	float UpperBodyAlpha = 1.0f;

	// ============== Aim Offset ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|AimOffset", meta=(AllowPrivateAccess="true"))
	float YawOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|AimOffset", meta=(AllowPrivateAccess="true"))
	float Pitch = 0.0f;

	// ============== Weapon IK ==============
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Anim|IK")
	float Hands_IK_Weight = 0.0f;

	// ============== Turn In Place ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float RootYawOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	FRotator MovingRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	FRotator LastMovingRotation = FRotator::ZeroRotator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float DistanceCurve = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	FName DistanceToPivot = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	FName Turning = NAME_None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float LastDistanceCurve = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float DeltaDistanceCurve = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float AbsRootYawOffset = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|TurnInPlace", meta=(AllowPrivateAccess="true"))
	float YawExcess = 0.0f;

	// ============== Equip Flags ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Equip", meta=(AllowPrivateAccess="true"))
	bool bIsPistolEquip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Equip", meta=(AllowPrivateAccess="true"))
	bool bIsRifleEquip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Equip", meta=(AllowPrivateAccess="true"))
	bool bIsShotgunEquip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Equip", meta=(AllowPrivateAccess="true"))
	bool bIsMeleeEquip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Equip", meta=(AllowPrivateAccess="true"))
	bool bIsGrenadeEquip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Equip", meta=(AllowPrivateAccess="true"))
	bool bIsPlaceItemEquip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Equip", meta=(AllowPrivateAccess="true"))
	bool bIsUsableItemEquip = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Equip", meta=(AllowPrivateAccess="true"))
	bool bIsWeaponEquip = false;

	// ============== Direction & Orientation ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Orientation", meta=(AllowPrivateAccess="true"))
	float F_OrientationAngle = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Orientation", meta=(AllowPrivateAccess="true"))
	float R_OrientationAngle = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Orientation", meta=(AllowPrivateAccess="true"))
	float B_OrientationAngle = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Orientation", meta=(AllowPrivateAccess="true"))
	float L_OrientationAngle = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Orientation", meta=(AllowPrivateAccess="true"))
	float DirectionAngle = 0.0f;

	// Нормализованный поворот по Yaw [-1..1]
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Camera", meta=(AllowPrivateAccess="true"))
	float TurnRate = 0.0f;

	// ============== Movement / BlendSpace ==============
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	float GroundSpeed = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	bool bShouldMove = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	bool bIsFalling = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	float Direction = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	EMoveDirection MoveDirection = EMoveDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	bool bIsCrouching = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	ELandState LandState = ELandState::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Anim|Movement", meta=(AllowPrivateAccess="true"))
	bool bIsJump = false;

	// ===== Anim getters (AnimInstance reads only) =====
	FORCEINLINE const FVector& GetAnimVelocity() const { return Velocity; }
	FORCEINLINE float GetAnimGroundSpeed() const { return GroundSpeed; }
	FORCEINLINE const FVector& GetAnimAcceleration() const { return Acceleration; }
	FORCEINLINE bool GetAnimShouldMove() const { return bShouldMove; }
	FORCEINLINE bool GetAnimIsFalling() const { return bIsFalling; }
	FORCEINLINE bool GetAnimIsCrouching() const { return bIsCrouching; }

	FORCEINLINE float GetAnimDirection() const { return Direction; }
	FORCEINLINE EMoveDirection GetAnimMoveDirection() const { return MoveDirection; }

	FORCEINLINE float GetFOrientationAngle() const { return F_OrientationAngle; }
	FORCEINLINE float GetROrientationAngle() const { return R_OrientationAngle; }
	FORCEINLINE float GetBOrientationAngle() const { return B_OrientationAngle; }
	FORCEINLINE float GetLOrientationAngle() const { return L_OrientationAngle; }

	FORCEINLINE float GetAnimDirectionAngle() const { return DirectionAngle; }
	FORCEINLINE float GetAnimTurnRate() const { return TurnRate; }

protected:
	void SetExhausted(bool bNewExhausted);
};
