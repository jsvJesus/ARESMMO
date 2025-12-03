#pragma once

#include "CoreMinimal.h"
#include "Animations/AnimType.h"
#include "ItemTypes.generated.h"

UENUM(BlueprintType)
enum class EHeroPartType : uint8
{
	None UMETA(DisplayName="None"),
	Head UMETA(DisplayName="Head"),
	Body UMETA(DisplayName="Body"),
	Legs UMETA(DisplayName="Legs")
};

UENUM(BlueprintType)
enum class EEquipmentSlotType : uint8
{
	None                 UMETA(DisplayName="None"),

	// Hero parts
	EquipmentSlotHead    UMETA(DisplayName="Head Part"),
	EquipmentSlotBody    UMETA(DisplayName="Body Part"),
	EquipmentSlotLegs    UMETA(DisplayName="Legs Part"),

	// Gear
	EquipmentSlotArmor   UMETA(DisplayName="Armor"),
	EquipmentSlotHelmet  UMETA(DisplayName="Helmet"),
	EquipmentSlotMask    UMETA(DisplayName="Mask"),
	EquipmentSlotBackpack UMETA(DisplayName="Backpack"),

	// Weapons
	EquipmentSlotWeapon1 UMETA(DisplayName="Weapon 1"),
	EquipmentSlotWeapon2 UMETA(DisplayName="Weapon 2"),
	EquipmentSlotPistol  UMETA(DisplayName="Pistol"),
	EquipmentSlotKnife   UMETA(DisplayName="Knife"),
	EquipmentSlotGrenade UMETA(DisplayName="Grenade"),

	// Usable
	EquipmentSlotDevice1 UMETA(DisplayName="Device 1"),
	EquipmentSlotDevice2 UMETA(DisplayName="Device 2")
};

/** Главная категория предмета */
UENUM(BlueprintType)
enum class EStoreCategory : uint8
{
	storecat_INVALID   UMETA(DisplayName="Invalid / None"),

	storecat_ACCOUNT   UMETA(DisplayName="Account"),
	storecat_Booster   UMETA(DisplayName="Booster"),
	storecat_HeroPack  UMETA(DisplayName="Hero Pack"),
	storecat_LootBox   UMETA(DisplayName="Loot Box"),

	storecat_Armor     UMETA(DisplayName="Armor"),
	storecat_Helmet    UMETA(DisplayName="Helmet"),
	storecat_Mask      UMETA(DisplayName="Mask"),
	storecat_Backpack  UMETA(DisplayName="Backpack"),
	storecat_HeroParts UMETA(DisplayName="Hero Parts"),

	storecat_ASR       UMETA(DisplayName="Assault Rifle"),
	storecat_SNP       UMETA(DisplayName="Sniper Rifle"),
	storecat_SHTG      UMETA(DisplayName="Shotgun"),
	storecat_HG        UMETA(DisplayName="Handgun"),
	storecat_MG        UMETA(DisplayName="Machine Gun"),
	storecat_SMG       UMETA(DisplayName="SMG"),
	storecat_MELEE     UMETA(DisplayName="Melee"),
	storecat_Grenade   UMETA(DisplayName="Grenade"),

	storecat_Medicine   UMETA(DisplayName="Medicine"),
	storecat_Food       UMETA(DisplayName="Food"),
	storecat_Water      UMETA(DisplayName="Water"),
	storecat_PlaceItem  UMETA(DisplayName="Placeable Item"),
	storecat_UsableItem UMETA(DisplayName="Usable Item"),

	storecat_Components   UMETA(DisplayName="Components"),
	storecat_CraftRecipes UMETA(DisplayName="Craft Recipes"),
	storecat_CraftItems   UMETA(DisplayName="Craft Items"),

	storecat_Ammo       UMETA(DisplayName="Ammo"),
	storecat_WeaponATTM UMETA(DisplayName="Weapon Attachments"),
	storecat_GearATTM   UMETA(DisplayName="Gear Attachments")
};

/* Подкатегории (одним enum-ом, но имена сгруппированы по категориям). */
UENUM(BlueprintType)
enum class EStoreSubCategory : uint8
{
	None UMETA(DisplayName="None"),

	// ===== storecat_ACCOUNT =====
	Account_VIP      UMETA(DisplayName="VIP"),
	Account_Premium  UMETA(DisplayName="Premium"),

	// ===== storecat_Booster =====
	Booster_DoubleXP UMETA(DisplayName="Double XP"),
	Booster_DoubleGD UMETA(DisplayName="Double GD"),

	// ===== storecat_HeroPack =====
	HeroPack_Killer    UMETA(DisplayName="УБИЙЦА"),
	HeroPack_Law       UMETA(DisplayName="ЗАКОННИК"),
	HeroPack_Tracker   UMETA(DisplayName="СЛЕДОПЫТ"),

	// ===== storecat_HeroParts =====
	HeroParts_Head     UMETA(DisplayName="Head"),
	HeroParts_Body     UMETA(DisplayName="Body"),
	HeroParts_Legs     UMETA(DisplayName="Legs"),

	// ===== storecat_Grenade =====
	Grenade_Frag       UMETA(DisplayName="Frag"),
	Grenade_Flash      UMETA(DisplayName="Flash"),
	Grenade_Smoke      UMETA(DisplayName="Smoke"),
	Grenade_ChemLight  UMETA(DisplayName="ChemLight"),

	// ===== storecat_PlaceItem =====
	Place_Barricade    UMETA(DisplayName="Barricade"),
	Place_Turret       UMETA(DisplayName="Turret"),

	// ===== storecat_UsableItem =====
	Usable_PDA         UMETA(DisplayName="PDA"),
	Usable_Detector    UMETA(DisplayName="Detector"),

	// ===== storecat_Ammo =====
	Ammo_223              UMETA(DisplayName=".223"),
	Ammo_45ACP_Hydroshock UMETA(DisplayName=".45 ACP Hydroshock"),
	Ammo_12x70_Buck       UMETA(DisplayName="12x70 Buck"),
	Ammo_12x70_Slug       UMETA(DisplayName="12x70 Slug"),
	Ammo_12x70_Dart       UMETA(DisplayName="12x70 Dart"),

	Ammo_545x39           UMETA(DisplayName="5.45x39"),
	Ammo_545x39_BP        UMETA(DisplayName="5.45x39 БП"),

	Ammo_556x45           UMETA(DisplayName="5.56x45"),
	Ammo_556x45_BP        UMETA(DisplayName="5.56x45 БП"),

	Ammo_57x28            UMETA(DisplayName="5.7x28"),
	Ammo_57x28_AP         UMETA(DisplayName="5.7x28 AP"),

	Ammo_762x54_7H14      UMETA(DisplayName="7.62x54 7Н14"),
	Ammo_762x54_7H1       UMETA(DisplayName="7.62x54 7Н1"),
	Ammo_762x54_BP        UMETA(DisplayName="7.62x54 БП"),
	Ammo_762x54_PP        UMETA(DisplayName="7.62x54 ПП"),

	Ammo_9x18             UMETA(DisplayName="9x18"),
	Ammo_9x18_PP          UMETA(DisplayName="9x18 +P+"),
	Ammo_9x18_PBP         UMETA(DisplayName="9x18 ПБП"),

	Ammo_9x19_FMJ         UMETA(DisplayName="9x19 FMJ"),
	Ammo_9x19_JHP         UMETA(DisplayName="9x19 JHP"),

	Ammo_9x39_PAB9        UMETA(DisplayName="9x39 ПАБ-9"),
	Ammo_9x39_SP5         UMETA(DisplayName="9x39 СП-5"),
	Ammo_9x39_SP6         UMETA(DisplayName="9x39 СП-6"),

	// ===== storecat_WeaponATTM =====
	WeaponATTM_Grip        UMETA(DisplayName="Grip"),
	WeaponATTM_Scope       UMETA(DisplayName="Scope"),
	WeaponATTM_Magazine    UMETA(DisplayName="Magazine"),
	WeaponATTM_Laser       UMETA(DisplayName="Laser"),
	WeaponATTM_Flashlight  UMETA(DisplayName="Flashlight"),
	WeaponATTM_Silencer    UMETA(DisplayName="Silencer / Compensator"),

	// ===== storecat_GearATTM =====
	GearATTM_NVG           UMETA(DisplayName="NVG"),
	GearATTM_Headlamp      UMETA(DisplayName="Headlamp"),
	GearATTM_Mask          UMETA(DisplayName="Mask Attachment")
};

/**
 * Класс предмета с точки зрения геймплея.
 * Это более абстрактный тип (для логики, инвентаря, применения).
 */
UENUM(BlueprintType)
enum class EItemClass : uint8
{
	None          UMETA(DisplayName="None"),

	Account       UMETA(DisplayName="Account / Meta"),
	Booster       UMETA(DisplayName="Booster"),
	HeroPack      UMETA(DisplayName="Hero Pack"),
	HeroPart      UMETA(DisplayName="Hero Part"),

	Armor         UMETA(DisplayName="Armor"),
	Helmet        UMETA(DisplayName="Helmet"),
	Mask          UMETA(DisplayName="Mask"),
	Backpack      UMETA(DisplayName="Backpack"),

	Weapon        UMETA(DisplayName="Weapon"),
	Grenade       UMETA(DisplayName="Grenade"),
	Melee         UMETA(DisplayName="Melee"),

	Consumable    UMETA(DisplayName="Consumable"),
	Medicine      UMETA(DisplayName="Medicine"),
	Food          UMETA(DisplayName="Food"),
	Water         UMETA(DisplayName="Water"),

	PlaceItem     UMETA(DisplayName="Placeable Item"),
	UsableItem    UMETA(DisplayName="Usable Item"),

	Component     UMETA(DisplayName="Component"),
	CraftRecipe   UMETA(DisplayName="Craft Recipe"),
	CraftItem     UMETA(DisplayName="Craft Item"),

	Ammo          UMETA(DisplayName="Ammo"),
	WeaponATTM    UMETA(DisplayName="Weapon Attachment"),
	GearATTM      UMETA(DisplayName="Gear Attachment")
};

static EEquipmentSlotType GetEquipmentSlotForCategory(EStoreCategory Category)
{
	switch (Category)
	{
	case EStoreCategory::storecat_Armor:    return EEquipmentSlotType::EquipmentSlotArmor;
	case EStoreCategory::storecat_Helmet:   return EEquipmentSlotType::EquipmentSlotHelmet;
	case EStoreCategory::storecat_Mask:     return EEquipmentSlotType::EquipmentSlotMask;
	case EStoreCategory::storecat_Backpack: return EEquipmentSlotType::EquipmentSlotBackpack;

	case EStoreCategory::storecat_ASR:
	case EStoreCategory::storecat_SNP:
	case EStoreCategory::storecat_SMG:
	case EStoreCategory::storecat_SHTG:
	case EStoreCategory::storecat_MG:
		return EEquipmentSlotType::EquipmentSlotWeapon1; // Weapon2 выбирается логикой

	case EStoreCategory::storecat_HG:     return EEquipmentSlotType::EquipmentSlotPistol;
	case EStoreCategory::storecat_MELEE:  return EEquipmentSlotType::EquipmentSlotKnife;
	case EStoreCategory::storecat_Grenade:return EEquipmentSlotType::EquipmentSlotGrenade;

		// Hero parts
	case EStoreCategory::storecat_HeroParts:
		// уточняется по StoreSubCategory
		return EEquipmentSlotType::None;

		// Usable items
	case EStoreCategory::storecat_UsableItem:
		return EEquipmentSlotType::EquipmentSlotDevice1;

	default:
		return EEquipmentSlotType::None;
	}
}

static inline EWeaponState GetWeaponStateForCategory(EStoreCategory Category)
{
	switch (Category)
	{
	case EStoreCategory::storecat_ASR:
	case EStoreCategory::storecat_SNP:
	case EStoreCategory::storecat_SMG:
		return EWeaponState::Rifle;

	case EStoreCategory::storecat_SHTG:
	case EStoreCategory::storecat_MG:
		return EWeaponState::Shotgun;

	case EStoreCategory::storecat_HG:
		return EWeaponState::Pistol;

	case EStoreCategory::storecat_MELEE:
		return EWeaponState::Melee;

	case EStoreCategory::storecat_Grenade:
		return EWeaponState::Grenade;

	case EStoreCategory::storecat_PlaceItem:
		return EWeaponState::PlaceItem;

	case EStoreCategory::storecat_UsableItem:
		return EWeaponState::UsableItem;

	default:
		return EWeaponState::Unarmed;
	}
}