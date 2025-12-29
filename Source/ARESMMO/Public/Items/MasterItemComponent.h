#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Items/MasterItemData.h"
#include "MasterItemComponent.generated.h"

USTRUCT(BlueprintType)
struct FItemInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ItemID = 0;

	/** Сколько в стаке (если стак используется) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Durability = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Charge = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CurrAmmo = 0;
};

USTRUCT(BlueprintType)
struct FResolvedItemConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) bool bUsesDurability = false;
	UPROPERTY(BlueprintReadOnly) int32 MaxDurability = 0;

	UPROPERTY(BlueprintReadOnly) bool bUsesCharge = false;
	UPROPERTY(BlueprintReadOnly) int32 MaxCharge = 0;

	UPROPERTY(BlueprintReadOnly) bool bUsesAmmo = false;
	UPROPERTY(BlueprintReadOnly) int32 MaxAmmo = 0;

	UPROPERTY(BlueprintReadOnly) FItemSize GridSize = FItemSize(1,1);
	UPROPERTY(BlueprintReadOnly) EEquipmentSlotType EquipmentSlot = EEquipmentSlotType::None;
	UPROPERTY(BlueprintReadOnly) EWeaponState WeaponState = EWeaponState::Unarmed;

	UPROPERTY(BlueprintReadOnly) EHeroPartType HeroPartType = EHeroPartType::None;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMasterItemChanged);

/**
 * Один компонент, который:
 * - хранит инстанс предмета (ItemID, количество, прочность, заряд, патроны)
 * - сам читает DataTable и резолвит "правильные" настройки (размер, слот, юзать ли прочность/заряд и т.д.)
 */
UCLASS(ClassGroup=(ARES), meta=(BlueprintSpawnableComponent))
class ARESMMO_API UMasterItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMasterItemComponent()
	{
		SetIsReplicatedByDefault(true);
		PrimaryComponentTick.bCanEverTick = false;
	}

	/** Событие: когда предмет/инстанс изменились */
	UPROPERTY(BlueprintAssignable, Category="ARES|Item")
	FOnMasterItemChanged OnItemChanged;

	/** Текущий инстанс предмета */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_ItemInstance, Category="ARES|Item")
	FItemInstanceData ItemInstance;

	/** Кэш резолва (не реплицируем) */
	UPROPERTY(Transient, BlueprintReadOnly, Category="ARES|Item")
	FResolvedItemConfig Resolved;

protected:
	virtual void BeginPlay() override
	{
		Super::BeginPlay();
		RebuildResolvedCache(/*bFixInstanceValues*/ true);
	}

	UFUNCTION()
	void OnRep_ItemInstance()
	{
		RebuildResolvedCache(/*bFixInstanceValues*/ true);
		OnItemChanged.Broadcast();
	}

public:
	/** Установить предмет по ItemID (подхватит дефолты из DT и правил) */
	UFUNCTION(BlueprintCallable, Category="ARES|Item")
	bool SetItemByID(int32 NewItemID, int32 NewQuantity = 1, bool bResetInstanceValues = true)
	{
		ItemInstance.ItemID = NewItemID;
		ItemInstance.Quantity = FMath::Max(1, NewQuantity);

		const bool bOk = RebuildResolvedCache(/*bFixInstanceValues*/ bResetInstanceValues);
		OnItemChanged.Broadcast();
		return bOk;
	}

	/** Получить строку DataTable (копией в BP) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	bool GetItemRow(FItemBaseRow& OutRow) const
	{
		const FItemBaseRow* Row = ItemDB::GetItemByID(ItemInstance.ItemID);
		if (!Row) return false;
		OutRow = *Row;
		return true;
	}

	/** Быстрые геттеры для UI */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	FResolvedItemConfig GetResolvedConfig() const { return Resolved; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	FItemSize GetGridSize() const { return Resolved.GridSize; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	EEquipmentSlotType GetResolvedEquipmentSlot() const { return Resolved.EquipmentSlot; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	EWeaponState GetResolvedWeaponState() const { return Resolved.WeaponState; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	EItemConditionState GetDurabilityState() const
	{
		if (!Resolved.bUsesDurability || Resolved.MaxDurability <= 0) return EItemConditionState::Perfect;
		return UItemConditionLibrary::GetConditionStateFromValues(ItemInstance.Durability, Resolved.MaxDurability);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	EItemConditionState GetChargeState() const
	{
		if (!Resolved.bUsesCharge || Resolved.MaxCharge <= 0) return EItemConditionState::Perfect;
		return UItemConditionLibrary::GetConditionStateFromValues(ItemInstance.Charge, Resolved.MaxCharge);
	}

	/** Нормализовать инстанс (обрезать значения по максимумам) */
	UFUNCTION(BlueprintCallable, Category="ARES|Item")
	void ClampInstanceValues()
	{
		if (Resolved.bUsesDurability && Resolved.MaxDurability > 0)
			ItemInstance.Durability = FMath::Clamp(ItemInstance.Durability, 0, Resolved.MaxDurability);

		if (Resolved.bUsesCharge && Resolved.MaxCharge > 0)
			ItemInstance.Charge = FMath::Clamp(ItemInstance.Charge, 0, Resolved.MaxCharge);

		if (Resolved.bUsesAmmo && Resolved.MaxAmmo > 0)
			ItemInstance.CurrAmmo = FMath::Clamp(ItemInstance.CurrAmmo, 0, Resolved.MaxAmmo);

		OnItemChanged.Broadcast();
	}

	/** Репликация */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME(UMasterItemComponent, ItemInstance);
	}

private:
	bool RebuildResolvedCache(bool bFixInstanceValues)
	{
		Resolved = FResolvedItemConfig();

		const FItemBaseRow* Row = ItemDB::GetItemByID(ItemInstance.ItemID);
		if (!Row)
		{
			// Ничего нет — сбрасываем безопасно
			if (bFixInstanceValues)
			{
				ItemInstance.Quantity = 1;
				ItemInstance.Durability = 0;
				ItemInstance.Charge = 0;
				ItemInstance.CurrAmmo = 0;
			}
			return false;
		}

		const UItemDatabaseSettings* Settings = GetDefault<UItemDatabaseSettings>();
		const bool bAuto = Settings ? Settings->bAutoFillDefaultsFromRules : true;

		// WeaponState
		Resolved.WeaponState = GetWeaponStateForCategory(Row->StoreCategory);

		// HeroPartType
		Resolved.HeroPartType = (Row->HeroPartType != EHeroPartType::None)
			? Row->HeroPartType
			: GetHeroPartTypeFromSubCategory(Row->StoreSubCategory);

		// EquipmentSlot
		if (Row->EquipmentSlot != EEquipmentSlotType::None)
		{
			Resolved.EquipmentSlot = Row->EquipmentSlot;
		}
		else
		{
			// HeroParts уточняем по типу
			if (Row->StoreCategory == EStoreCategory::storecat_HeroParts || Row->ItemClass == EItemClass::HeroPart)
			{
				switch (Resolved.HeroPartType)
				{
				case EHeroPartType::Head: Resolved.EquipmentSlot = EEquipmentSlotType::EquipmentSlotHead; break;
				case EHeroPartType::Body: Resolved.EquipmentSlot = EEquipmentSlotType::EquipmentSlotBody; break;
				case EHeroPartType::Legs: Resolved.EquipmentSlot = EEquipmentSlotType::EquipmentSlotLegs; break;
				default:                  Resolved.EquipmentSlot = EEquipmentSlotType::None; break;
				}
			}
			else
			{
				Resolved.EquipmentSlot = GetEquipmentSlotForCategory(Row->StoreCategory);
			}
		}

		// GridSize: если в DT оставили дефолт 1x1, а для класса дефолт другой — подставляем.
		Resolved.GridSize = Row->GridSize;
		const FItemSize ClassDefault = UItemSizeRules::GetDefaultSize(Row->ItemClass);

		const bool bRowSizeLooksDefault =
			(Resolved.GridSize.Width <= 0 || Resolved.GridSize.Height <= 0) ||
			(Resolved.GridSize.Width == 1 && Resolved.GridSize.Height == 1 && !(ClassDefault.Width == 1 && ClassDefault.Height == 1));

		if (bAuto && bRowSizeLooksDefault)
		{
			Resolved.GridSize = ClassDefault;
		}

		// Durability
		Resolved.bUsesDurability = Row->bUseDurability || (Row->MaxDurability > 0);
		if (bAuto) Resolved.bUsesDurability = Resolved.bUsesDurability || UItemConfigRules::UsesDurability(Row->StoreCategory);

		Resolved.MaxDurability = Row->MaxDurability;
		if (Resolved.bUsesDurability && Resolved.MaxDurability <= 0 && bAuto)
		{
			Resolved.MaxDurability = Settings ? Settings->DefaultMaxDurabilityIfMissing : 100;
		}

		// Charge
		Resolved.bUsesCharge = Row->bUseCharge || (Row->MaxCharge > 0);
		if (bAuto) Resolved.bUsesCharge = Resolved.bUsesCharge || UItemConfigRules::UsesCharge(Row->StoreSubCategory);

		Resolved.MaxCharge = Row->MaxCharge;
		if (Resolved.bUsesCharge && Resolved.MaxCharge <= 0 && bAuto)
		{
			Resolved.MaxCharge = Settings ? Settings->DefaultMaxChargeIfMissing : 100;
		}

		// Ammo
		Resolved.bUsesAmmo = Row->bUseAmmo || (Row->MaxAmmo > 0);
		Resolved.MaxAmmo = Row->MaxAmmo;

		// Если просили — выставляем/чинем инстанс значения по DT/правилам
		if (bFixInstanceValues)
		{
			ItemInstance.Quantity = FMath::Max(1, ItemInstance.Quantity);

			if (Resolved.bUsesDurability)
			{
				const int32 DefaultD = (Row->DefaultDurability > 0) ? Row->DefaultDurability : Resolved.MaxDurability;
				if (ItemInstance.Durability <= 0) ItemInstance.Durability = DefaultD;
				ItemInstance.Durability = FMath::Clamp(ItemInstance.Durability, 0, Resolved.MaxDurability);
			}
			else
			{
				ItemInstance.Durability = 0;
			}

			if (Resolved.bUsesCharge)
			{
				const int32 DefaultC = (Row->DefaultCharge > 0) ? Row->DefaultCharge : Resolved.MaxCharge;
				if (ItemInstance.Charge <= 0) ItemInstance.Charge = DefaultC;
				ItemInstance.Charge = FMath::Clamp(ItemInstance.Charge, 0, Resolved.MaxCharge);
			}
			else
			{
				ItemInstance.Charge = 0;
			}

			if (Resolved.bUsesAmmo)
			{
				const int32 DefaultAmmo = (Row->CurrAmmo > 0) ? Row->CurrAmmo : 0;
				if (ItemInstance.CurrAmmo < 0) ItemInstance.CurrAmmo = DefaultAmmo;
				if (Resolved.MaxAmmo > 0)
					ItemInstance.CurrAmmo = FMath::Clamp(ItemInstance.CurrAmmo, 0, Resolved.MaxAmmo);
			}
			else
			{
				ItemInstance.CurrAmmo = 0;
			}
		}

		return true;
	}
};