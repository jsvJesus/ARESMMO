#pragma once

#include "CoreMinimal.h"
#include "ItemSize.h"
#include "Engine/DataTable.h"
#include "ItemTypes.h"
#include "ItemData.generated.h"

class AWeaponBase;
class UWeaponAttachmentBase;

USTRUCT(BlueprintType)
struct FConsumableEffects
{
	GENERATED_BODY()

	/** Сколько отхилить/нанести по здоровью (+хил, -урон) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float HealthDelta = 0.0f;

	/** Сколько восстановить/сжечь стамину (+восп, -сжечь) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float StaminaDelta = 0.0f;

	/** Изменение голода: - снижает голод, + увеличивает (0..100) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float FoodDelta = 0.0f;

	/** Изменение жажды: - снижает жажду, + увеличивает (0..100) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float WaterDelta = 0.0f;

	/** Радиоактивность: + даёт радиацию, - очищает */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float RadiationDelta = 0.0f;

	/** Токсическое заражение (Biohazard) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float BiohazardDelta = 0.0f;

	/** Пси-радиация (PsyRad) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float PsyRadDelta = 0.0f;

	/** Отравление (Poisoning) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float PoisoningDelta = 0.0f;

	/** Cold — заморозка/переохлаждение от этого предмета (например, холодная вода зимой) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float ColdDelta = 0.0f;

	/**
	 * Шанс плохого эффекта (0..100).
	 * Например, прокисшая еда/грязная вода:
	 *   - если прокнуло — добавим доп. Poisoning/Biohazard и т.п.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float BadEffectChance = 0.0f;

	/** Доп. сила плохого эффекта, если прокнул BadEffectChance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	float BadEffectPower = 0.0f;
};

/**
 * Базовая строка таблицы предметов.
 * На этом этапе — общая информация, без специфичных статов оружия и т.п.
 */
USTRUCT(BlueprintType)
struct ARESMMO_API FItemBaseRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	/** Уникальный числовой ID предмета */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	int32 ItemID = 0;

	/** Внутреннее имя (для удобства, можно использовать как строковый ID) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	FName InternalName;

	/** Локализуемое название предмета */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	FText DisplayName;

	/** Описание предмета */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	FText Description;

	/** Главная категория (storecat_*) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	EStoreCategory StoreCategory = EStoreCategory::storecat_INVALID;

	/** Подкатегория (по желанию) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	EStoreSubCategory StoreSubCategory = EStoreSubCategory::None;

	/** Геймплейный класс предмета (для логики инвентаря и применения) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	EItemClass ItemClass = EItemClass::None;

	/** Эффекты при употреблении (Medicine/Food/Water) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable")
	FConsumableEffects ConsumableEffects;

	/** Использует ли предмет стак */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	bool bUseStack = false;
	
	/** Максимальный размер стака (если bUseStack=true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item",
		meta=(ClampMin="1", EditCondition="bUseStack", EditConditionHides))
	int32 MaxStackSize = 1;

	/** Использует ли предмет вес */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	bool bUseWeight = true;
	
	/** Вес одного предмета (если bUseWeight=true) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item",
		meta=(ClampMin="0.0", EditCondition="bUseWeight", EditConditionHides))
	float Weight = 0.0f;

	/** Бонус к максимальному переносимому весу, если предмет экипирован (например рюкзак) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	float CarryWeightBonus = 0.0f;

	///////////////////////////////////////////////////////
	
	FORCEINLINE int32 GetEffectiveMaxStackSize() const
	{
		return bUseStack ? FMath::Max(1, MaxStackSize) : 1;
	}

	FORCEINLINE float GetUnitWeightKg() const
	{
		return bUseWeight ? FMath::Max(0.f, Weight) : 0.f;
	}

	///////////////////////////////////////////////////////

	/** Иконка для инвентаря / магазина */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	TSoftObjectPtr<UTexture2D> Icon;
	
	/** Размер предмета в сеточном инвентаре */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	FItemSize GridSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	USkeletalMesh* PreviewMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	UStaticMesh* PreviewStaticMesh = nullptr;

	/** Класс актора в мире (pickup / placeable), если нужен */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World")
	TSoftClassPtr<AActor> WorldActorClass;

	/** Можно ли продавать предмет */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy")
	bool bCanSell = true;

	/** Можно ли покупать предмет */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy")
	bool bCanBuy = true;

	/** Цена в игровой валюте (GD) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy")
	int32 PriceGameDollars = 0;

	/** Цена в донатной валюте (GC) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy")
	int32 PriceGameCoins = 0;
	
	/** Использует ли предмет ремонт */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability")
    bool bUseDurability = false;

	/** Максимальная прочность */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability")
	int32 MaxDurability = 0;

	/** Текущая прочность при создании */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability")
	int32 DefaultDurability = 0;

	/** Использует ли предмет заряд */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge")
	bool bUseCharge = false;

	/** Максимальный заряд */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge")
	int32 MaxCharge = 0;

	/** Заряд по умолчанию */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge")
	int32 DefaultCharge = 0;

	/** Использует ли предмет патроны (обычно только WeaponATTM_Magazine) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo")
	bool bUseAmmo = false;

	/** Максимум патронов в магазине */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo")
	int32 MaxAmmo = 0;

	/** Текущее кол-во патронов (инстанс в инвентаре) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo")
	int32 CurrAmmo = 0;

	/** Какой тип патронов принимает магазин (Ammo_*). Если None — примет любой. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo",
		meta=(EditCondition="bUseAmmo", EditConditionHides))
	EStoreSubCategory AcceptedAmmoSubCategory = EStoreSubCategory::None;

	/** Конкретный тип магазина (AK_Mag_30, STANAG_Mag_60 и т.д.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo",
		meta=(EditCondition="bUseAmmo", EditConditionHides))
	EWeaponMagazineType MagazineType = EWeaponMagazineType::None;

	/** Класс логики для WeaponATTM (Magazine/Grip/Scope/...) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponAttachment")
	TSubclassOf<UWeaponAttachmentBase> WeaponATTMClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHeroPartType HeroPartType = EHeroPartType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* HeroPartMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEquipmentSlotType EquipmentSlot = EEquipmentSlotType::None;

	/* ===================== WEAPONS (Actor-based) ===================== */

	/** Класс оружия (Actor), который будет спавниться при экипировке */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	TSoftClassPtr<AWeaponBase> WeaponActorClass;

	/** Override сокета для экипировки (если пусто — берём по слоту: Rifle_Socket / Pistol_Socket / weapon_r fallback) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	FName WeaponEquipSocketName = NAME_None;

	/* ===================== WEAPON ATTACHMENTS ===================== */

	/** Mesh для WeaponATTM (если это attachment). Можно StaticMesh (приоритет) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponAttachment")
	UStaticMesh* WeaponAttachmentStaticMesh = nullptr;

	/** Mesh для WeaponATTM (если это attachment). Если StaticMesh пуст — можно SkeletalMesh */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponAttachment")
	USkeletalMesh* WeaponAttachmentSkeletalMesh = nullptr;
};

// === Глобальные функции доступа к базе предметов ===
class UDataTable;

namespace ItemDB
{
	// Ленивая загрузка DataTable с предметами
	ARESMMO_API UDataTable* GetItemsDataTable();

	// Поиск строки по ItemID
	ARESMMO_API const FItemBaseRow* GetItemByID(int32 ItemID);
}