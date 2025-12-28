#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponAttachments.h"
#include "ARESMMO/ARESMMOCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

static bool IsWeaponATTMSubCategory(EStoreSubCategory SubCat)
{
	switch (SubCat)
	{
	case EStoreSubCategory::WeaponATTM_Magazine:
	case EStoreSubCategory::WeaponATTM_Grip:
	case EStoreSubCategory::WeaponATTM_Scope:
	case EStoreSubCategory::WeaponATTM_Laser:
	case EStoreSubCategory::WeaponATTM_Flashlight:
	case EStoreSubCategory::WeaponATTM_Silencer:
	case EStoreSubCategory::WeaponATTM_Module:
		return true;
	default:
		return false;
	}
}

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetGenerateOverlapEvents(false);

	// На будущее (сеть)
	bReplicates = true;
	SetReplicateMovement(false);
}

void AWeaponBase::SetOwningCharacter(AARESMMOCharacter* NewOwner)
{
	OwningCharacter = NewOwner;
	SetOwner(NewOwner);
}

void AWeaponBase::Fire()
{
	UE_LOG(LogTemp, Log, TEXT("Weapon Fire: %s"), *GetName());
}

void AWeaponBase::Reload()
{
	UE_LOG(LogTemp, Log, TEXT("Weapon Reload: %s"), *GetName());
}

void AWeaponBase::StartAim()
{
	UE_LOG(LogTemp, Log, TEXT("Weapon Aim Start: %s"), *GetName());
}

void AWeaponBase::StopAim()
{
	UE_LOG(LogTemp, Log, TEXT("Weapon Aim Stop: %s"), *GetName());
}

FName AWeaponBase::GetSocketForAttachment(EStoreSubCategory AttachmentSubCategory) const
{
	for (const FWeaponAttachmentSlotDef& Def : AttachmentSlots)
	{
		if (Def.AttachmentSubCategory == AttachmentSubCategory)
		{
			return Def.SocketName;
		}
	}
	return NAME_None;
}

bool AWeaponBase::GetHandIKTransforms_World(FTransform& OutL, FTransform& OutR) const
{
	OutL = FTransform::Identity;
	OutR = FTransform::Identity;

	if (!OwningCharacter || !OwningCharacter->GetMesh())
		return false;

	USkeletalMeshComponent* CharMesh = OwningCharacter->GetMesh();

	const bool bHasL = CharMesh->DoesSocketExist(LeftHandIKSocket);
	const bool bHasR = CharMesh->DoesSocketExist(RightHandIKSocket);

	if (!bHasL)
	{
		// Не ломаем анимацию: просто вернём false и IK вес останется 0
		UE_LOG(LogTemp, Warning,
			TEXT("Weapon %s: missing LEFT IK socket on CHARACTER (%s). CharacterMesh=%s"),
			*GetName(),
			*LeftHandIKSocket.ToString(),
			*GetNameSafe(CharMesh->GetSkeletalMeshAsset())
		);
		return false;
	}

	OutL = CharMesh->GetSocketTransform(LeftHandIKSocket, RTS_World);

	// Right socket не обязателен
	if (bHasR)
	{
		OutR = CharMesh->GetSocketTransform(RightHandIKSocket, RTS_World);
	}

	return true;
}

bool AWeaponBase::CheckWeaponATTMClass(const FItemBaseRow& ItemRow, FString& OutFailReason) const
{
	OutFailReason.Empty();

	if (ItemRow.StoreCategory != EStoreCategory::storecat_WeaponATTM)
	{
		OutFailReason = TEXT("Item is not storecat_WeaponATTM");
		return false;
	}

	if (!IsWeaponATTMSubCategory(ItemRow.StoreSubCategory))
	{
		OutFailReason = FString::Printf(TEXT("Invalid WeaponATTM StoreSubCategory: %d"), (int32)ItemRow.StoreSubCategory);
		return false;
	}

	if (!ItemRow.WeaponATTMClass)
	{
		OutFailReason = TEXT("WeaponATTMClass is not set in ItemRow (DataTable).");
		return false;
	}

	return true;
}

UWeaponAttachmentBase* AWeaponBase::CreateAttachmentLogicFromItemRow(const FItemBaseRow& ItemRow,
	FString& OutFailReason) const
{
	OutFailReason.Empty();

	if (!CheckWeaponATTMClass(ItemRow, OutFailReason))
	{
		return nullptr;
	}

	UWeaponAttachmentBase* Logic = NewObject<UWeaponAttachmentBase>(const_cast<AWeaponBase*>(this), ItemRow.WeaponATTMClass);
	if (!Logic)
	{
		OutFailReason = TEXT("Failed to create WeaponATTMClass instance.");
		return nullptr;
	}

	// Защита: класс логики должен соответствовать подкатегории предмета
	if (Logic->HandledSubCategory != EStoreSubCategory::None &&
		Logic->HandledSubCategory != ItemRow.StoreSubCategory)
	{
		OutFailReason = FString::Printf(
			TEXT("WeaponATTMClass HandledSubCategory mismatch. Logic=%d, ItemRow=%d"),
			(int32)Logic->HandledSubCategory,
			(int32)ItemRow.StoreSubCategory
		);
		return nullptr;
	}

	return Logic;
}

bool AWeaponBase::HasAttachment(EStoreSubCategory SubCategory) const
{
	if (const FAttachedWeaponATTM* Found = AttachedATTM.Find(SubCategory))
	{
		return Found->bValid;
	}
	return false;
}

bool AWeaponBase::CanAttachATTMToWeapon(const FItemBaseRow& ItemRow, FString& OutFailReason) const
{
	OutFailReason.Empty();

	// базовая валидность + WeaponATTMClass
	if (!CheckWeaponATTMClass(ItemRow, OutFailReason))
	{
		return false;
	}

	// Запрет 2 одинаковых (по StoreSubCategory)
	if (HasAttachment(ItemRow.StoreSubCategory))
	{
		OutFailReason = TEXT("Attachment of this type is already installed (duplicate).");
		return false;
	}

	// Special: Scope на АК через dovetail (Module)
	if (ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Scope)
	{
		// если есть верхняя планка — ок
		if (bHasTopRail)
		{
			// ok
		}
		else
		{
			// верхней планки нет — прицел можно только через dovetail на АК с установленным Module
			const bool bCanViaDovetail = bIsAKFamilyWeapon && bSupportsDovetailSideMount && bDovetailRailEnabled;
			if (!bCanViaDovetail)
			{
				OutFailReason = TEXT("Scope requires TopRail OR (AK + Dovetail Module installed).");
				return false;
			}
		}
	}

	// Module — только для АК (и только если поддерживает боковую ласточку)
	if (ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Module)
	{
		if (!bIsAKFamilyWeapon || !bSupportsDovetailSideMount)
		{
			OutFailReason = TEXT("Module (dovetail) can be attached only to AK-family weapons that support side mount.");
			return false;
		}
	}

	// Создаём логику и даём ей ещё один шанс (по желанию — дополнительные проверки в классах)
	{
		FString Reason;
		UWeaponAttachmentBase* Logic = CreateAttachmentLogicFromItemRow(ItemRow, Reason);
		if (!Logic)
		{
			OutFailReason = Reason;
			return false;
		}

		if (!Logic->CanAttachToWeapon(this, ItemRow))
		{
			OutFailReason = TEXT("WeaponATTMClass::CanAttachToWeapon returned false.");
			return false;
		}
	}

	return true;
}

bool AWeaponBase::AttachATTMToWeapon(const FItemBaseRow& ItemRow, FString& OutFailReason)
{
	OutFailReason.Empty();

	// общая проверка (включая duplicates и dovetail)
	if (!CanAttachATTMToWeapon(ItemRow, OutFailReason))
	{
		return false;
	}

	// создать логику
	UWeaponAttachmentBase* Logic = CreateAttachmentLogicFromItemRow(ItemRow, OutFailReason);
	if (!Logic)
	{
		return false;
	}

	// выполнить attach
	if (!Logic->AttachToWeapon(this, ItemRow))
	{
		OutFailReason = TEXT("WeaponATTMClass::AttachToWeapon failed.");
		return false;
	}

	// сохранить
	FAttachedWeaponATTM NewSlot;
	NewSlot.Set(ItemRow, Logic);
	AttachedATTM.Add(ItemRow.StoreSubCategory, NewSlot);

	// Module включает dovetail rail
	if (ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Module)
	{
		bDovetailRailEnabled = true;
	}

	return true;
}

bool AWeaponBase::DetachATTMFromWeapon(EStoreSubCategory SubCategory, FItemBaseRow& OutDetachedItemRow,
	FString& OutFailReason)
{
	OutFailReason.Empty();

	FAttachedWeaponATTM* Found = AttachedATTM.Find(SubCategory);
	if (!Found || !Found->bValid)
	{
		OutFailReason = TEXT("No attachment in that slot.");
		return false;
	}

	// Защита: нельзя снять Module, если Scope держится только на dovetail (нет TopRail)
	if (SubCategory == EStoreSubCategory::WeaponATTM_Module)
	{
		const bool bScopeInstalled = HasAttachment(EStoreSubCategory::WeaponATTM_Scope);
		const bool bScopeNeedsModule = bScopeInstalled && !bHasTopRail; // если верхней планки нет, прицел был возможен только через dovetail
		if (bScopeNeedsModule)
		{
			OutFailReason = TEXT("Cannot detach Dovetail Module while a Scope is installed and there is no TopRail. Detach Scope first.");
			return false;
		}
	}

	// вызвать Detach у логики
	if (Found->Logic && !Found->Logic->DetachFromWeapon(this))
	{
		OutFailReason = TEXT("WeaponATTMClass::DetachFromWeapon failed.");
		return false;
	}

	OutDetachedItemRow = Found->ItemRow;

	// убрать из слота
	AttachedATTM.Remove(SubCategory);

	// выключить dovetail rail если сняли Module
	if (SubCategory == EStoreSubCategory::WeaponATTM_Module)
	{
		bDovetailRailEnabled = false;
	}

	return true;
}

bool AWeaponBase::CanAcceptAttachment(const FItemBaseRow& AttachmentRow) const
{
	if (AttachmentRow.StoreCategory != EStoreCategory::storecat_WeaponATTM)
		return false;

	const FName Socket = GetSocketForAttachment(AttachmentRow.StoreSubCategory);
	if (Socket.IsNone())
		return false;

	// Можно расширить правилами (совместимость, список допустимых, калибр и т.д.)
	return true;
}

bool AWeaponBase::AttachItem(const FItemBaseRow& AttachmentRow)
{
	if (!WeaponMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: WeaponMesh is null."));
		return false;
	}

	// базовые проверки + WeaponATTMClass
	FString FailReason;
	if (!CheckWeaponATTMClass(AttachmentRow, FailReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: %s | Item=%s"),
			*FailReason, *AttachmentRow.InternalName.ToString());
		return false;
	}

	// должен быть слот/сокет под эту подкатегорию
	if (!CanAcceptAttachment(AttachmentRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: Weapon has no socket for this attachment type. | Item=%s Sub=%d"),
			*AttachmentRow.InternalName.ToString(), (int32)AttachmentRow.StoreSubCategory);
		return false;
	}

	// основная ATTM-логика
	if (!AttachATTMToWeapon(AttachmentRow, FailReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: AttachATTMToWeapon failed: %s | Item=%s"),
			*FailReason, *AttachmentRow.InternalName.ToString());
		return false;
	}

	// визуал (меш) — опционально (Module может быть без меша)
	const FName Socket = GetSocketForAttachment(AttachmentRow.StoreSubCategory);

	// на всякий случай (если вдруг визуал остался)
	DetachAttachment(AttachmentRow.StoreSubCategory);

	bool bVisualOk = true;

	if (AttachmentRow.WeaponAttachmentStaticMesh)
	{
		UStaticMeshComponent* StaticComp = NewObject<UStaticMeshComponent>(this);
		if (!StaticComp)
		{
			bVisualOk = false;
		}
		else
		{
			StaticComp->SetStaticMesh(AttachmentRow.WeaponAttachmentStaticMesh);
			StaticComp->AttachToComponent(WeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
			StaticComp->RegisterComponent();
			RuntimeAttachments.Add(AttachmentRow.StoreSubCategory, StaticComp);
		}
	}
	else if (AttachmentRow.WeaponAttachmentSkeletalMesh)
	{
		USkeletalMeshComponent* SkelComp = NewObject<USkeletalMeshComponent>(this);
		if (!SkelComp)
		{
			bVisualOk = false;
		}
		else
		{
			SkelComp->SetSkeletalMesh(AttachmentRow.WeaponAttachmentSkeletalMesh);
			SkelComp->AttachToComponent(WeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
			SkelComp->RegisterComponent();
			RuntimeAttachments.Add(AttachmentRow.StoreSubCategory, SkelComp);
		}
	}
	else
	{
		// без меша — это нормально (например Module на первом этапе)
		bVisualOk = true;
	}

	// Если визуал упал — откатываем логику, чтобы не оставлять “невидимый” аттач случайно
	if (!bVisualOk)
	{
		FItemBaseRow DummyRow;
		FString DetachReason;
		DetachATTMFromWeapon(AttachmentRow.StoreSubCategory, DummyRow, DetachReason);

		UE_LOG(LogTemp, Warning, TEXT("AttachItem: Visual component creation failed. Rolled back. | Item=%s"),
			*AttachmentRow.InternalName.ToString());
		return false;
	}

	return true;
}

bool AWeaponBase::DetachItem(EStoreSubCategory SubCategory, FItemBaseRow& OutDetachedItemRow, FString& OutFailReason)
{
	OutFailReason.Empty();

	// снять логику (проверка “нельзя снять Module если Scope держится на dovetail” — внутри)
	if (!DetachATTMFromWeapon(SubCategory, OutDetachedItemRow, OutFailReason))
	{
		return false;
	}

	// снять визуал
	DetachAttachment(SubCategory);

	return true;
}

bool AWeaponBase::DetachAttachment(EStoreSubCategory AttachmentSubCategory)
{
	if (TObjectPtr<USceneComponent>* Found = RuntimeAttachments.Find(AttachmentSubCategory))
	{
		if (USceneComponent* Comp = *Found)
		{
			Comp->DestroyComponent();
		}
		RuntimeAttachments.Remove(AttachmentSubCategory);
		return true;
	}
	return false;
}
