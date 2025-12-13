#pragma once

#include "CoreMinimal.h"
#include "ItemSize.h"
#include "Engine/DataTable.h"
#include "ItemTypes.h"
#include "ItemData.generated.h"

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|tem")
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

	/** Максимальный размер стака */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	bool bUseStackSize = false;

	// Текущее количество в стаке
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	int32 StackSize = 1;

	/** Вес одного предмета (для системы веса / переноса) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item")
	float Weight = 0.0f;

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

	/** Можно ли продавать обратно в магазин */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy")
	bool bCanSell = true;

	/** Цена в мягкой валюте (GD, например) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy")
	int32 PriceGD = 0;

	/** Цена в донатной валюте (GC, например) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy")
	int32 PriceGC = 0;

	/** Можно ли ремонтировать предметы */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability")
	bool bUseRepair = false;

	/** Максимальная прочность */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability")
	int32 MaxDurability = 0;

	/** Стандартная прочность */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability")
	int32 DefaultDurability = 0;

	/** Текущая прочность */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability")
	int32 CurrDurability = 0;

	/** Использует ли предмет заряд */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge")
	bool bUseCharge = false;

	/** Максимальный заряд */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge")
	int32 MaxCharge = 0;

	/** Стандартный заряд */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge")
	int32 DefaultCharge = 0;

	/** Текущий заряд */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge")
	int32 CurrCharge = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHeroPartType HeroPartType = EHeroPartType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* HeroPartMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEquipmentSlotType EquipmentSlot = EEquipmentSlotType::None;
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