#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/SoftObjectPtr.h"
#include "MasterItemData.generated.h"

// Forward declarations
class UTexture2D;
class USkeletalMesh;
class UStaticMesh;
class AActor;
class AWeaponBase;
class UWeaponAttachmentBase;

/* ===================== ANIM TYPES ===================== */
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Unarmed    UMETA(DisplayName="Unarmed"),
	Rifle      UMETA(DisplayName="Rifle"),
	Shotgun    UMETA(DisplayName="Shotgun"),
	Pistol     UMETA(DisplayName="Pistol"),
	Melee      UMETA(DisplayName="Melee"),
	Grenade    UMETA(DisplayName="Grenade"),
	PlaceItem  UMETA(DisplayName="Place Item"),
	UsableItem UMETA(DisplayName="Usable Item")
};

UENUM(BlueprintType)
enum class EMoveDirection : uint8
{
	None     UMETA(DisplayName="None"),
	Forward  UMETA(DisplayName="Forward"),
	Backward UMETA(DisplayName="Backward"),
	Left     UMETA(DisplayName="Left"),
	Right    UMETA(DisplayName="Right")
};

UENUM(BlueprintType)
enum class ELandState : uint8
{
	None   UMETA(DisplayName="None"),
	Normal UMETA(DisplayName="Normal"),
	Soft   UMETA(DisplayName="Soft"),
	Hard   UMETA(DisplayName="Hard")
};

/* ===================== ITEM CONDITION ===================== */
UENUM(BlueprintType)
enum class EItemConditionState : uint8
{
	Broken   UMETA(DisplayName="0-1% (Black)"),
	Bad      UMETA(DisplayName="2-24% (Red)"),
	Low      UMETA(DisplayName="25-49% (Orange)"),
	Medium   UMETA(DisplayName="50-74% (Yellow)"),
	Good     UMETA(DisplayName="75-99% (Green)"),
	Perfect  UMETA(DisplayName="100% (Blue)")
};

UCLASS()
class ARESMMO_API UItemConditionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Расчёт состояния по текущему и максимальному значению (прочность или заряд) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	static EItemConditionState GetConditionStateFromValues(int32 CurrentValue, int32 MaxValue)
	{
		if (MaxValue <= 0) return EItemConditionState::Perfect;

		const int32 ClampedCurrent = FMath::Clamp(CurrentValue, 0, MaxValue);
		const float Pct = (MaxValue > 0) ? (100.0f * (float)ClampedCurrent / (float)MaxValue) : 100.0f;

		if (Pct >= 100.0f) return EItemConditionState::Perfect;
		if (Pct >= 75.0f)  return EItemConditionState::Good;
		if (Pct >= 50.0f)  return EItemConditionState::Medium;
		if (Pct >= 25.0f)  return EItemConditionState::Low;
		if (Pct >= 2.0f)   return EItemConditionState::Bad;
		return EItemConditionState::Broken; // 0..1%
	}

	/** Цвет для состояния (UI) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	static FLinearColor GetConditionColor(EItemConditionState State)
	{
		switch (State)
		{
		case EItemConditionState::Broken:  return FLinearColor::Black;
		case EItemConditionState::Bad:     return FLinearColor(1.f, 0.f, 0.f, 1.f);
		case EItemConditionState::Low:     return FLinearColor(1.f, 0.45f, 0.f, 1.f);
		case EItemConditionState::Medium:  return FLinearColor(1.f, 1.f, 0.f, 1.f);
		case EItemConditionState::Good:    return FLinearColor(0.f, 1.f, 0.f, 1.f);
		case EItemConditionState::Perfect: return FLinearColor(0.f, 0.6f, 1.f, 1.f);
		default:                           return FLinearColor::White;
		}
	}
};

/* ===================== ITEM SIZE ===================== */
USTRUCT(BlueprintType)
struct FItemSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Height = 1;

	FItemSize() {}
	FItemSize(int32 W, int32 H) : Width(W), Height(H) {}
};

/* ===================== ITEM TYPES ===================== */
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
	None                  UMETA(DisplayName="None"),

	// Hero parts
	EquipmentSlotHead     UMETA(DisplayName="Head Part"),
	EquipmentSlotBody     UMETA(DisplayName="Body Part"),
	EquipmentSlotLegs     UMETA(DisplayName="Legs Part"),

	// Gear
	EquipmentSlotArmor    UMETA(DisplayName="Armor"),
	EquipmentSlotHelmet   UMETA(DisplayName="Helmet"),
	EquipmentSlotMask     UMETA(DisplayName="Mask"),
	EquipmentSlotBackpack UMETA(DisplayName="Backpack"),

	// Weapons
	EquipmentSlotWeapon1  UMETA(DisplayName="Weapon 1"),
	EquipmentSlotWeapon2  UMETA(DisplayName="Weapon 2"),
	EquipmentSlotPistol   UMETA(DisplayName="Pistol"),
	EquipmentSlotKnife    UMETA(DisplayName="Knife"),
	EquipmentSlotGrenade  UMETA(DisplayName="Grenade"),

	// Usable
	EquipmentSlotDevice1  UMETA(DisplayName="Device 1"),
	EquipmentSlotDevice2  UMETA(DisplayName="Device 2")
};

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
	WeaponATTM_Silencer    UMETA(DisplayName="Muzzle"),
	WeaponATTM_Module      UMETA(DisplayName="Module"),

	// ===== storecat_GearATTM =====
	GearATTM_NVG           UMETA(DisplayName="NVG"),
	GearATTM_Headlamp      UMETA(DisplayName="Headlamp"),
	GearATTM_Mask          UMETA(DisplayName="Mask Attachment"),
	GearATTM_Belt          UMETA(DisplayName="Gear Belt"),
	GearATTM_Plate         UMETA(DisplayName="Gear Plate"),

	// ===== storecat_Components =====
	Item_Battery           UMETA(DisplayName="Battery"),
	Item_RapairKit         UMETA(DisplayName="Rapair Kit")
};

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

UENUM(BlueprintType)
enum class EWeaponMagazineType : uint8
{
	None UMETA(DisplayName="None"),

	AK_Mag_30   UMETA(DisplayName="AK Mag 30"),
	AK_Mag_45   UMETA(DisplayName="AK Mag 45"),
	AK_Mag_60   UMETA(DisplayName="AK Mag 60"),

	AKM_Mag_30  UMETA(DisplayName="AKM Mag 30"),
	AKM_Mag_45  UMETA(DisplayName="AKM Mag 45"),
	AKM_Mag_60  UMETA(DisplayName="AKM Mag 60"),

	STANAG_Mag_30  UMETA(DisplayName="STANAG 30"),
	STANAG_Mag_60  UMETA(DisplayName="STANAG 60"),
	STANAG_Mag_100 UMETA(DisplayName="STANAG 100"),
};

// Совместимость: оставляем те же функции, но делаем их header-safe (inline).
static FORCEINLINE EEquipmentSlotType GetEquipmentSlotForCategory(EStoreCategory Category)
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
		return EEquipmentSlotType::EquipmentSlotWeapon1;

	case EStoreCategory::storecat_HG:      return EEquipmentSlotType::EquipmentSlotPistol;
	case EStoreCategory::storecat_MELEE:   return EEquipmentSlotType::EquipmentSlotKnife;
	case EStoreCategory::storecat_Grenade: return EEquipmentSlotType::EquipmentSlotGrenade;

	case EStoreCategory::storecat_HeroParts:
		return EEquipmentSlotType::None;

	case EStoreCategory::storecat_UsableItem:
		return EEquipmentSlotType::EquipmentSlotDevice1;

	default:
		return EEquipmentSlotType::None;
	}
}

static FORCEINLINE EWeaponState GetWeaponStateForCategory(EStoreCategory Category)
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

static FORCEINLINE EHeroPartType GetHeroPartTypeFromSubCategory(EStoreSubCategory SubCat)
{
	switch (SubCat)
	{
	case EStoreSubCategory::HeroParts_Head: return EHeroPartType::Head;
	case EStoreSubCategory::HeroParts_Body: return EHeroPartType::Body;
	case EStoreSubCategory::HeroParts_Legs: return EHeroPartType::Legs;
	default:                                return EHeroPartType::None;
	}
}

/* ===================== RULES ===================== */
UCLASS()
class ARESMMO_API UItemSizeRules : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item|Rules")
	static FItemSize GetDefaultSize(EItemClass Class)
	{
		switch (Class)
		{
		case EItemClass::HeroPart:   return FItemSize(2, 2);
		case EItemClass::Armor:      return FItemSize(2, 2);
		case EItemClass::Helmet:     return FItemSize(2, 2);
		case EItemClass::Mask:       return FItemSize(2, 2);
		case EItemClass::Backpack:   return FItemSize(2, 2);

		case EItemClass::Weapon:     return FItemSize(4, 2);
		case EItemClass::Grenade:    return FItemSize(1, 1);
		case EItemClass::Melee:      return FItemSize(2, 2);

		case EItemClass::Consumable: return FItemSize(1, 1);
		case EItemClass::Medicine:   return FItemSize(1, 1);
		case EItemClass::Food:       return FItemSize(1, 1);
		case EItemClass::Water:      return FItemSize(1, 1);

		case EItemClass::PlaceItem:  return FItemSize(2, 2);
		case EItemClass::UsableItem: return FItemSize(1, 2);

		case EItemClass::Component:  return FItemSize(1, 2);
		case EItemClass::CraftRecipe:return FItemSize(2, 2);
		case EItemClass::CraftItem:  return FItemSize(2, 2);

		case EItemClass::Ammo:       return FItemSize(1, 1);
		case EItemClass::WeaponATTM: return FItemSize(1, 2);
		case EItemClass::GearATTM:   return FItemSize(2, 1);
		default:                     return FItemSize(1, 1);
		}
	}
};

UCLASS()
class ARESMMO_API UItemConfigRules : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item|Rules")
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

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item|Rules")
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

/* ===================== DATABASE SETTINGS ===================== */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Item Database"))
class ARESMMO_API UItemDatabaseSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Сюда в Project Settings назначаешь DT_Items (DataTable на FItemBaseRow). */
	UPROPERTY(Config, EditAnywhere, Category="Database")
	TSoftObjectPtr<UDataTable> ItemsDataTable;

	/** Если true — Master код будет подставлять дефолты по правилам, когда в DT значения не заполнены. */
	UPROPERTY(Config, EditAnywhere, Category="Database")
	bool bAutoFillDefaultsFromRules = true;

	/** Дефолтные Max значения, если в DataTable пусто, но предмет "по правилам" должен иметь прочность/заряд. */
	UPROPERTY(Config, EditAnywhere, Category="Database")
	int32 DefaultMaxDurabilityIfMissing = 100;

	UPROPERTY(Config, EditAnywhere, Category="Database")
	int32 DefaultMaxChargeIfMissing = 100;
};

/* ===================== DATA TABLE ROW ===================== */
USTRUCT(BlueprintType)
struct FConsumableEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float HealthDelta = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float StaminaDelta = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float FoodDelta = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float WaterDelta = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float RadiationDelta = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float BiohazardDelta = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float PsyRadDelta = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float PoisoningDelta = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float ColdDelta = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float BadEffectChance = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") float BadEffectPower  = 0.0f;
};

USTRUCT(BlueprintType)
struct ARESMMO_API FItemBaseRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") int32 ItemID = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") FName InternalName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") EStoreCategory StoreCategory = EStoreCategory::storecat_INVALID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") EStoreSubCategory StoreSubCategory = EStoreSubCategory::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") EItemClass ItemClass = EItemClass::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Consumable") FConsumableEffects ConsumableEffects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") bool bUseStack = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item",
		meta=(ClampMin="1", EditCondition="bUseStack", EditConditionHides))
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") bool bUseWeight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item",
		meta=(ClampMin="0.0", EditCondition="bUseWeight", EditConditionHides))
	float Weight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Item") float CarryWeightBonus = 0.0f;

	FORCEINLINE int32 GetEffectiveMaxStackSize() const { return bUseStack ? FMath::Max(1, MaxStackSize) : 1; }
	FORCEINLINE float GetUnitWeightKg() const { return bUseWeight ? FMath::Max(0.f, Weight) : 0.f; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI") TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI") FItemSize GridSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI") USkeletalMesh* PreviewMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI") UStaticMesh* PreviewStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="World") TSoftClassPtr<AActor> WorldActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy") bool bCanSell = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy") bool bCanBuy = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy") int32 PriceGameDollars = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Economy") int32 PriceGameCoins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability") bool bUseDurability = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability") int32 MaxDurability = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Durability") int32 DefaultDurability = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge") bool bUseCharge = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge") int32 MaxCharge = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Charge") int32 DefaultCharge = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo") bool bUseAmmo = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo") int32 MaxAmmo = 0;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo") int32 CurrAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo", meta=(EditCondition="bUseAmmo", EditConditionHides))
	EStoreSubCategory AcceptedAmmoSubCategory = EStoreSubCategory::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Config|Ammo", meta=(EditCondition="bUseAmmo", EditConditionHides))
	EWeaponMagazineType MagazineType = EWeaponMagazineType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponAttachment")
	TSubclassOf<UWeaponAttachmentBase> WeaponATTMClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) EHeroPartType HeroPartType = EHeroPartType::None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) USkeletalMesh* HeroPartMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) EEquipmentSlotType EquipmentSlot = EEquipmentSlotType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	TSoftClassPtr<AWeaponBase> WeaponActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	FName WeaponEquipSocketName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponAttachment")
	UStaticMesh* WeaponAttachmentStaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="WeaponAttachment")
	USkeletalMesh* WeaponAttachmentSkeletalMesh = nullptr;
};

/* ===================== ITEM DB ACCESS (INLINE) ===================== */
namespace ItemDB
{
	static FORCEINLINE const UItemDatabaseSettings* GetSettings()
	{
		return GetDefault<UItemDatabaseSettings>();
	}

	static FORCEINLINE UDataTable* GetItemsDataTable()
	{
		static TWeakObjectPtr<UDataTable> Cached;
		if (Cached.IsValid())
		{
			return Cached.Get();
		}

		const UItemDatabaseSettings* Settings = GetSettings();
		if (Settings && Settings->ItemsDataTable.IsValid())
		{
			Cached = Settings->ItemsDataTable.Get();
			return Cached.Get();
		}

		if (Settings && !Settings->ItemsDataTable.IsNull())
		{
			UDataTable* Loaded = Settings->ItemsDataTable.LoadSynchronous();
			Cached = Loaded;
			return Loaded;
		}

		UE_LOG(LogTemp, Warning, TEXT("[ItemDB] ItemsDataTable is not set in Project Settings -> Item Database."));
		return nullptr;
	}

	static FORCEINLINE const FItemBaseRow* GetItemByID(int32 ItemID)
	{
		UDataTable* DT = GetItemsDataTable();
		if (!DT || ItemID <= 0) return nullptr;

		// Кэш ID->RowName на конкретную таблицу
		static TWeakObjectPtr<UDataTable> CachedFor;
		static TMap<int32, FName> IdToRow;

		if (CachedFor.Get() != DT)
		{
			CachedFor = DT;
			IdToRow.Reset();

			const TMap<FName, uint8*>& RowMap = DT->GetRowMap();
			for (const TPair<FName, uint8*>& Pair : RowMap)
			{
				const FItemBaseRow* Row = reinterpret_cast<const FItemBaseRow*>(Pair.Value);
				if (!Row) continue;

				if (Row->ItemID <= 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("[ItemDB] Row '%s' has invalid ItemID (%d)."), *Pair.Key.ToString(), Row->ItemID);
					continue;
				}

				if (IdToRow.Contains(Row->ItemID))
				{
					UE_LOG(LogTemp, Warning, TEXT("[ItemDB] Duplicate ItemID %d in DataTable (row '%s')."), Row->ItemID, *Pair.Key.ToString());
					continue;
				}

				IdToRow.Add(Row->ItemID, Pair.Key);
			}
		}

		if (const FName* RowName = IdToRow.Find(ItemID))
		{
			return DT->FindRow<FItemBaseRow>(*RowName, TEXT("ItemDB::GetItemByID"));
		}

		return nullptr;
	}
}