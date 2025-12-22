#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/ItemData.h"
#include "WeaponBase.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class USceneComponent;
class AARESMMOCharacter;

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
	bool AttachItem(const FItemBaseRow& AttachmentRow);

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	bool DetachAttachment(EStoreSubCategory AttachmentSubCategory);

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	FName GetSocketForAttachment(EStoreSubCategory AttachmentSubCategory) const;

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

	// ===== IK sockets on WeaponMesh =====
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
};