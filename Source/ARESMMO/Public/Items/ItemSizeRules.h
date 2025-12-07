#pragma once

#include "CoreMinimal.h"
#include "ItemSize.h"
#include "ItemTypes.h"
#include "ItemSizeRules.generated.h"

/**
 * Предопределённые размеры для разных ItemClass.
 * Для классов, где есть несколько размеров, сетать в DataTable вручную.
 */
UCLASS()
class ARESMMO_API UItemSizeRules : public UObject
{
	GENERATED_BODY()

public:
	static FItemSize GetDefaultSize(const EItemClass Class)
	{
		switch (Class)
		{
			case EItemClass::HeroPart:   return FItemSize(2,2);
			case EItemClass::Armor:      return FItemSize(2,2);
			case EItemClass::Helmet:     return FItemSize(2,2);
			case EItemClass::Mask:       return FItemSize(2,2);
			case EItemClass::Backpack:   return FItemSize(2,2);

			case EItemClass::Weapon:     return FItemSize(2,4);
			case EItemClass::Grenade:    return FItemSize(1,1);
			case EItemClass::Melee:      return FItemSize(2,2);

			case EItemClass::Consumable: return FItemSize(1,1);
			case EItemClass::Medicine:   return FItemSize(1,1);
			case EItemClass::Food:       return FItemSize(1,1);
			case EItemClass::Water:      return FItemSize(1,1);

			case EItemClass::PlaceItem:  return FItemSize(2,2);
			case EItemClass::UsableItem: return FItemSize(1,2);

			case EItemClass::Component:  return FItemSize(1,2);
			case EItemClass::CraftRecipe:return FItemSize(2,2);
			case EItemClass::CraftItem:  return FItemSize(2,2);

			case EItemClass::Ammo:       return FItemSize(1,1);
			case EItemClass::WeaponATTM: return FItemSize(1,2);
			case EItemClass::GearATTM:   return FItemSize(2,1);
		default: ;
		}

		return FItemSize(1,1);
	}
};