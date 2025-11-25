#pragma once

#include "CoreMinimal.h"
#include "ItemSize.h"
#include "Engine/DataTable.h"
#include "ItemTypes.h"
#include "ItemData.generated.h"

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	int32 ItemID = 0;

	/** Внутреннее имя (для удобства, можно использовать как строковый ID) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FName InternalName;

	/** Локализуемое название предмета */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FText DisplayName;

	/** Описание предмета */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FText Description;

	/** Главная категория (storecat_*) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EStoreCategory StoreCategory = EStoreCategory::storecat_INVALID;

	/** Подкатегория (по желанию) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EStoreSubCategory StoreSubCategory = EStoreSubCategory::None;

	/** Геймплейный класс предмета (для логики инвентаря и применения) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	EItemClass ItemClass = EItemClass::None;

	/** Максимальный размер стака */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	int32 MaxStackSize = 1;

	/** Вес одного предмета (для системы веса / переноса) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
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

	/** Цена в мягкой валюте (GD, например) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy")
	int32 PriceGD = 0;

	/** Цена в донатной валюте (GC, например) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy")
	int32 PriceGC = 0;

	/** Можно ли продавать обратно в магазин */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy")
	bool bCanSell = true;

	/** Максимальная прочность */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	int32 MaxDurability = 0;

	/** Текущая прочность при создании */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	int32 DefaultDurability = 0;

	/** Использует ли предмет заряд */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	bool bUseCharge = false;

	/** Максимальный заряд */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	int32 MaxCharge = 0;

	/** Заряд по умолчанию */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config")
	int32 DefaultCharge = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHeroPartType HeroPartType = EHeroPartType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USkeletalMesh* HeroPartMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEquipmentSlotType EquipmentSlot = EEquipmentSlotType::None;
};