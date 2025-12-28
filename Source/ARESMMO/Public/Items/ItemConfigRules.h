#pragma once

#include "CoreMinimal.h"
#include "ItemTypes.h"
#include "UObject/Object.h"
#include "ItemConfigRules.generated.h"

UCLASS()
class UItemConfigRules : public UObject
{
	GENERATED_BODY()

public:

	static bool UsesDurability(EStoreCategory Cat)
	{
		switch (Cat)
		{
		case EStoreCategory::storecat_Armor:
		case EStoreCategory::storecat_Helmet:

		case EStoreCategory::storecat_ASR:
		case EStoreCategory::storecat_SNP:
		case EStoreCategory::storecat_SHTG:
		case EStoreCategory::storecat_HG:
		case EStoreCategory::storecat_MG:
		case EStoreCategory::storecat_SMG:
		case EStoreCategory::storecat_MELEE:

		case EStoreCategory::storecat_CraftItems:
		case EStoreCategory::storecat_PlaceItem:
		case EStoreCategory::storecat_UsableItem:
			return true;

		default:
			return false;
		}
	}

	static bool UsesCharge(EStoreSubCategory SubCat)
	{
		switch (SubCat)
		{
		case EStoreSubCategory::WeaponATTM_Laser:
		case EStoreSubCategory::WeaponATTM_Flashlight:
		case EStoreSubCategory::GearATTM_NVG:
		case EStoreSubCategory::GearATTM_Headlamp:
		case EStoreSubCategory::Usable_Detector:
		case EStoreSubCategory::Usable_PDA:
			return true;

		default:
			return false;
		}
	}
};