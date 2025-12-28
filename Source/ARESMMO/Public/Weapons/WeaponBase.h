#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/ItemData.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class USceneComponent;
class UWeaponAttachmentBase;
class AARESMMOCharacter;

USTRUCT(BlueprintType)
struct FAttachedWeaponATTM
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FItemBaseRow ItemRow;

	UPROPERTY(Transient)
	TObjectPtr<UWeaponAttachmentBase> Logic = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bValid = false;

	void Set(const FItemBaseRow& InRow, UWeaponAttachmentBase* InLogic)
	{
		ItemRow = InRow;
		Logic = InLogic;
		bValid = true;
	}

	void Clear()
	{
		Logic = nullptr;
		bValid = false;
	}
};

USTRUCT(BlueprintType)
struct FWeaponAttachmentSlotDef
{
	GENERATED_BODY()

	/** Какая подкатегория отвечает за этот слот (WeaponATTM_Grip / WeaponATTM_Scope и т.д.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponAttachment")
	EStoreSubCategory AttachmentSubCategory = EStoreSubCategory::None;

	/** В какой сокет на WeaponMesh цеплять */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponAttachment")
	FName SocketName = NAME_None;
};

UCLASS(Blueprintable)
class ARESMMO_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	void SetOwningCharacter(AARESMMOCharacter* NewOwner);

	/* ===== Base gameplay API (пока заглушки, расширишь) ===== */
	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	virtual void Fire();

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	virtual void Reload();

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	virtual void StartAim();

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon")
	virtual void StopAim();

	/* ===== Attachments ===== */
	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	bool CanAcceptAttachment(const FItemBaseRow& AttachmentRow) const;

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	bool DetachAttachment(EStoreSubCategory AttachmentSubCategory);

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	FName GetSocketForAttachment(EStoreSubCategory AttachmentSubCategory) const;

	// Один аттач на один StoreSubCategory (Scope/Grip/Laser/...)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Attachments")
	TMap<EStoreSubCategory, FAttachedWeaponATTM> AttachedATTM;

	// ===== Mount / Platform flags (минимум для логики "ласточкин хвост") =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Mounts")
	bool bIsAKFamilyWeapon = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Mounts")
	bool bHasTopRail = false; // Пикатинни сверху (если true — прицел можно без модуля)

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon|Mounts")
	bool bSupportsDovetailSideMount = false; // у АК обычно true

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon|Mounts")
	bool bDovetailRailEnabled = false; // станет true когда установлен Module

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Weapon")
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Weapon")
	USkeletalMeshComponent* WeaponMesh = nullptr;

	/** Настройка сокетов слотов аттачей (в BP_Weapon) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Weapon|Attachment")
	TArray<FWeaponAttachmentSlotDef> AttachmentSlots;

	/** Реально созданные компоненты аттачей в рантайме */
	UPROPERTY(Transient)
	TMap<EStoreSubCategory, TObjectPtr<USceneComponent>> RuntimeAttachments;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Weapon")
	TObjectPtr<AARESMMOCharacter> OwningCharacter = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Weapon|Attach")
	FName CharacterAttachSocket = TEXT("weapon_r");

	// ===== IK sockets on CHARACTER mesh (NOT on WeaponMesh) =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Weapon|IK")
	FName LeftHandIKSocket = TEXT("Hand_L_Socket");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Weapon|IK")
	FName RightHandIKSocket = TEXT("Hand_R_Socket");

	UFUNCTION(BlueprintPure, Category="ARES|Weapon|IK")
	FName GetLeftHandIKSocket() const { return LeftHandIKSocket; }

	UFUNCTION(BlueprintPure, Category="ARES|Weapon|IK")
	FName GetRightHandIKSocket() const { return RightHandIKSocket; }

public:
	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|IK")
	bool GetHandIKTransforms_World(FTransform& OutL, FTransform& OutR) const;

	UFUNCTION(BlueprintPure, Category="ARES|Weapon|Attach")
	FName GetCharacterAttachSocket() const { return CharacterAttachSocket; }

	/* ===== Attachments ===== */
	// Отдельная проверка ItemRow.WeaponATTMClass
	UFUNCTION(BlueprintCallable, Category="Weapon|Attachments")
	bool CheckWeaponATTMClass(const FItemBaseRow& ItemRow, FString& OutFailReason) const;

	// СreateAttachmentLogicFromItemRow()
	UFUNCTION(BlueprintCallable, Category="Weapon|Attachments")
	UWeaponAttachmentBase* CreateAttachmentLogicFromItemRow(const FItemBaseRow& ItemRow, FString& OutFailReason) const;

	// helper
	UFUNCTION(BlueprintCallable, Category="Weapon|Attachments")
	bool HasAttachment(EStoreSubCategory SubCategory) const;

	// Основная проверка, включая "2 одинаковых нельзя" + dovetail rule
	UFUNCTION(BlueprintCallable, Category="Weapon|Attachments")
	bool CanAttachATTMToWeapon(const FItemBaseRow& ItemRow, FString& OutFailReason) const;

	// AttachATTMToWeapon()
	UFUNCTION(BlueprintCallable, Category="Weapon|Attachments")
	bool AttachATTMToWeapon(const FItemBaseRow& ItemRow, FString& OutFailReason);

	// DetachATTMFromWeapon()
	UFUNCTION(BlueprintCallable, Category="Weapon|Attachments")
	bool DetachATTMFromWeapon(EStoreSubCategory SubCategory, FItemBaseRow& OutDetachedItemRow, FString& OutFailReason);

	UFUNCTION(BlueprintCallable, Category="ARES|Attachment")
	bool AttachItem(const FItemBaseRow& ItemRow);
};