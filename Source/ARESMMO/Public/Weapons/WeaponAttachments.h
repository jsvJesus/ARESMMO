#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Items/ItemData.h"
#include "WeaponAttachments.generated.h"

class AWeaponBase;

/**
 * База для всей логики WeaponATTM.
 * Идея: в DataTable у каждого аттача указан WeaponATTMClass, и оружие/персонаж вызывает функции этого класса.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ARESMMO_API UWeaponAttachmentBase : public UObject
{
	GENERATED_BODY()

public:
	/** Какой StoreSubCategory обслуживает этот класс (Grip/Scope/Magazine/...) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attachment")
	EStoreSubCategory HandledSubCategory = EStoreSubCategory::None;

	/** Можно ли прицепить к конкретному оружию (пока базовая заглушка) */
	UFUNCTION(BlueprintCallable, Category="Attachment|CanAtach")
	virtual bool CanAttachToWeapon(const AWeaponBase* Weapon, const FItemBaseRow& AttachmentItem) const
	{
		(void)Weapon;
		(void)AttachmentItem;
		return true;
	}

	/** Прицепить (пока заглушка — тут потом сделать attach mesh к сокету оружия) */
	UFUNCTION(BlueprintCallable, Category="Attachment|Attach")
	virtual bool AttachToWeapon(AWeaponBase* Weapon, const FItemBaseRow& AttachmentItem)
	{
		(void)Weapon;
		(void)AttachmentItem;
		return true;
	}

	/** Отцепить (заглушка) */
	UFUNCTION(BlueprintCallable, Category="Attachment|Detach")
	virtual bool DetachFromWeapon(AWeaponBase* Weapon)
	{
		(void)Weapon;
		return true;
	}
};

/** ===================== MAGAZINE ===================== */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ARESMMO_API UWeaponATTM_Magazine : public UWeaponAttachmentBase
{
	GENERATED_BODY()

public:
	UWeaponATTM_Magazine()
	{
		HandledSubCategory = EStoreSubCategory::WeaponATTM_Magazine;
	}

	UFUNCTION(BlueprintCallable, Category="Attachment|Magazine")
	bool CanAcceptAmmoType(const FItemBaseRow& MagItem, EStoreSubCategory AmmoType) const
	{
		if (!MagItem.bUseAmmo) return false;
		if (MagItem.AcceptedAmmoSubCategory == EStoreSubCategory::None) return true; // fallback (лучше не использовать)
		return MagItem.AcceptedAmmoSubCategory == AmmoType;
	}

	UFUNCTION(BlueprintCallable, Category="Attachment|Magazine")
	int32 GetFreeSpace(const FItemBaseRow& MagItem) const
	{
		if (!MagItem.bUseAmmo || MagItem.MaxAmmo <= 0) return 0;
		return FMath::Max(0, MagItem.MaxAmmo - MagItem.CurrAmmo);
	}

	/** Загрузить патроны в магазин, вернёт сколько реально загрузили */
	UFUNCTION(BlueprintCallable, Category="Attachment|Magazine")
	int32 LoadAmmo(FItemBaseRow& MagItem, EStoreSubCategory AmmoType, int32 Amount)
	{
		if (Amount <= 0) return 0;
		if (!CanAcceptAmmoType(MagItem, AmmoType)) return 0;

		const int32 Free = GetFreeSpace(MagItem);
		const int32 Loaded = FMath::Clamp(Amount, 0, Free);
		MagItem.CurrAmmo += Loaded;
		return Loaded;
	}

	/** Выгрузить патроны, вернёт сколько реально выгрузили */
	UFUNCTION(BlueprintCallable, Category="Attachment|Magazine")
	int32 UnloadAmmo(FItemBaseRow& MagItem, int32 Amount)
	{
		if (Amount <= 0) return 0;
		if (!MagItem.bUseAmmo) return 0;

		const int32 Unloaded = FMath::Clamp(Amount, 0, MagItem.CurrAmmo);
		MagItem.CurrAmmo -= Unloaded;
		return Unloaded;
	}
};

/** ===================== GRIP ===================== */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ARESMMO_API UWeaponATTM_Grip : public UWeaponAttachmentBase
{
	GENERATED_BODY()

public:
	UWeaponATTM_Grip()
	{
		HandledSubCategory = EStoreSubCategory::WeaponATTM_Grip;
	}

	// сюда потом: модификаторы отдачи/раскачки и т.д.
};

/** ===================== SCOPE ===================== */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ARESMMO_API UWeaponATTM_Scope : public UWeaponAttachmentBase
{
	GENERATED_BODY()

public:
	UWeaponATTM_Scope()
	{
		HandledSubCategory = EStoreSubCategory::WeaponATTM_Scope;
	}

	// сюда потом: зум/прицеливание/линза/ретикл
};

/** ===================== LASER ===================== */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ARESMMO_API UWeaponATTM_Laser : public UWeaponAttachmentBase
{
	GENERATED_BODY()

public:
	UWeaponATTM_Laser()
	{
		HandledSubCategory = EStoreSubCategory::WeaponATTM_Laser;
	}

	// сюда потом: включение/выключение лазера (и расход заряда)
};

/** ===================== FLASHLIGHT ===================== */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ARESMMO_API UWeaponATTM_Flashlight : public UWeaponAttachmentBase
{
	GENERATED_BODY()

public:
	UWeaponATTM_Flashlight()
	{
		HandledSubCategory = EStoreSubCategory::WeaponATTM_Flashlight;
	}

	// сюда потом: включение/выключение фонаря (и расход заряда)
};

/** ===================== SILENCER ===================== */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ARESMMO_API UWeaponATTM_Silencer : public UWeaponAttachmentBase
{
	GENERATED_BODY()

public:
	UWeaponATTM_Silencer()
	{
		HandledSubCategory = EStoreSubCategory::WeaponATTM_Silencer;
	}

	// сюда потом: модификация звука/вспышки/разброса
};

/** ===================== MODULE (ласточкин хвост / адаптер) ===================== */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class ARESMMO_API UWeaponATTM_Module : public UWeaponAttachmentBase
{
	GENERATED_BODY()

public:
	UWeaponATTM_Module()
	{
		HandledSubCategory = EStoreSubCategory::WeaponATTM_Module;
	}

	// идея: этот модуль даёт “площадку” под Scope для АК (ласточкин хвост)
	// потом: в CanAttachToWeapon проверять семейство оружия, а в AttachToWeapon включать “optic rail enabled”
};