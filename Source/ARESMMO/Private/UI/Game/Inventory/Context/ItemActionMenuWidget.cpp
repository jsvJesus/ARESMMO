#include "UI/Game/Inventory/Context/ItemActionMenuWidget.h"
#include "UI/Game/Inventory/InventoryWidget.h"
#include "Components/Button.h"

void UItemActionMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Equip)          Btn_Equip->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickEquip);
	if (Btn_Attach)         Btn_Attach->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickAttach);
	if (Btn_Detach)         Btn_Detach->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDetach);
	if (Btn_Detach_Grip)    Btn_Detach_Grip->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDetachGrip);
	if (Btn_Detach_Scope)   Btn_Detach_Scope->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDetachScope);
	if (Btn_Detach_Magazine) Btn_Detach_Magazine->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDetachMagazine);
	if (Btn_Detach_Laser)   Btn_Detach_Laser->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDetachLaser);
	if (Btn_Detach_Flashlight) Btn_Detach_Flashlight->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDetachFlashlight);
	if (Btn_Detach_Silencer) Btn_Detach_Silencer->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDetachSilencer);
	if (Btn_Detach_Module)  Btn_Detach_Module->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDetachModule);
	if (Btn_Use)            Btn_Use->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickUse);
	if (Btn_Study)          Btn_Study->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickStudy);
	if (Btn_Drop)           Btn_Drop->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickDrop);
	if (Btn_ChargeItem)     Btn_ChargeItem->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickChargeItem);
	if (Btn_ChargeMagazine) Btn_ChargeMagazine->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickChargeMagazine);
	if (Btn_Repair)         Btn_Repair->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickRepair);
}

void UItemActionMenuWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (OwnerInventoryWidget.IsValid())
	{
		OwnerInventoryWidget->CancelCloseItemActionMenu();
	}
}

void UItemActionMenuWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (OwnerInventoryWidget.IsValid())
	{
		OwnerInventoryWidget->RequestCloseItemActionMenu();
	}
}

void UItemActionMenuWidget::SetBtnVisible(UButton* Btn, bool bVisible)
{
	if (!Btn) return;
	Btn->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UItemActionMenuWidget::SetBtnEnabled(UButton* Btn, bool bEnabled)
{
	if (!Btn) return;
	Btn->SetIsEnabled(bEnabled);
}

void UItemActionMenuWidget::Fire(EItemContextAction Action)
{
	OnActionSelected.Broadcast(Action);
}

void UItemActionMenuWidget::SetDetachButtonsVisible(bool bVisible)
{
	SetBtnVisible(Btn_Detach_Grip, bVisible && CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Grip));
	SetBtnVisible(Btn_Detach_Scope, bVisible && CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Scope));
	SetBtnVisible(Btn_Detach_Magazine, bVisible && CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Magazine));
	SetBtnVisible(Btn_Detach_Laser, bVisible && CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Laser));
	SetBtnVisible(Btn_Detach_Flashlight, bVisible && CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Flashlight));
	SetBtnVisible(Btn_Detach_Silencer, bVisible && CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Silencer));
	SetBtnVisible(Btn_Detach_Module, bVisible && CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Module));
}

void UItemActionMenuWidget::SetDetachButtonVisibleFor(EStoreSubCategory SubCategory, bool bVisible)
{
	switch (SubCategory)
	{
	case EStoreSubCategory::WeaponATTM_Grip:
		SetBtnVisible(Btn_Detach_Grip, bVisible);
		break;
	case EStoreSubCategory::WeaponATTM_Scope:
		SetBtnVisible(Btn_Detach_Scope, bVisible);
		break;
	case EStoreSubCategory::WeaponATTM_Magazine:
		SetBtnVisible(Btn_Detach_Magazine, bVisible);
		break;
	case EStoreSubCategory::WeaponATTM_Laser:
		SetBtnVisible(Btn_Detach_Laser, bVisible);
		break;
	case EStoreSubCategory::WeaponATTM_Flashlight:
		SetBtnVisible(Btn_Detach_Flashlight, bVisible);
		break;
	case EStoreSubCategory::WeaponATTM_Silencer:
		SetBtnVisible(Btn_Detach_Silencer, bVisible);
		break;
	case EStoreSubCategory::WeaponATTM_Module:
		SetBtnVisible(Btn_Detach_Module, bVisible);
		break;
	default:
		break;
	}
}

void UItemActionMenuWidget::QueueDetachAction(EStoreSubCategory SubCategory)
{
	PendingDetachSubCategory = SubCategory;
	Fire(EItemContextAction::Detach);
}

void UItemActionMenuWidget::SetupForItem(const FItemBaseRow& ItemRow, bool bHasBattery, bool bHasAmmo, bool bHasRepairKit)
{
	// Equip
	const bool bEquip =
		ItemRow.StoreCategory == EStoreCategory::storecat_Armor ||
		ItemRow.StoreCategory == EStoreCategory::storecat_Helmet ||
		ItemRow.StoreCategory == EStoreCategory::storecat_Mask ||
		ItemRow.StoreCategory == EStoreCategory::storecat_Backpack ||
		ItemRow.StoreCategory == EStoreCategory::storecat_HeroParts ||
		ItemRow.StoreCategory == EStoreCategory::storecat_ASR ||
		ItemRow.StoreCategory == EStoreCategory::storecat_SNP ||
		ItemRow.StoreCategory == EStoreCategory::storecat_SHTG ||
		ItemRow.StoreCategory == EStoreCategory::storecat_HG ||
		ItemRow.StoreCategory == EStoreCategory::storecat_MG ||
		ItemRow.StoreCategory == EStoreCategory::storecat_SMG ||
		ItemRow.StoreCategory == EStoreCategory::storecat_MELEE ||
		ItemRow.StoreCategory == EStoreCategory::storecat_Grenade ||
		(ItemRow.StoreCategory == EStoreCategory::storecat_UsableItem &&
		 (ItemRow.StoreSubCategory == EStoreSubCategory::Usable_PDA ||
		  ItemRow.StoreSubCategory == EStoreSubCategory::Usable_Detector));

	SetBtnVisible(Btn_Equip, bEquip);

	// Attach (для предметов-аттачей)
	const bool bAttach =
		ItemRow.StoreCategory == EStoreCategory::storecat_WeaponATTM ||
		ItemRow.StoreCategory == EStoreCategory::storecat_GearATTM;

	SetBtnVisible(Btn_Attach, bAttach);

	// Detach (для weapon attachments: снимаем модуль соответствующего типа)
	const bool bDetach = ItemRow.StoreCategory == EStoreCategory::storecat_WeaponATTM;
	SetBtnVisible(Btn_Detach, bDetach);
	SetDetachButtonsVisible(false);

	// Use item
	const bool bUse =
		ItemRow.StoreCategory == EStoreCategory::storecat_Medicine ||
		ItemRow.StoreCategory == EStoreCategory::storecat_Food ||
		ItemRow.StoreCategory == EStoreCategory::storecat_Water ||
		ItemRow.StoreCategory == EStoreCategory::storecat_PlaceItem;

	SetBtnVisible(Btn_Use, bUse);

	// Learn
	const bool bStudy = (ItemRow.StoreCategory == EStoreCategory::storecat_CraftRecipes);
	SetBtnVisible(Btn_Study, bStudy);

	// Drop — всегда
	SetBtnVisible(Btn_Drop, true);

	// Charge Item (battery items)
	const bool bChargeItemVisible =
		ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Laser ||
		ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Flashlight ||
		ItemRow.StoreSubCategory == EStoreSubCategory::GearATTM_NVG ||
		ItemRow.StoreSubCategory == EStoreSubCategory::GearATTM_Headlamp ||
		ItemRow.StoreSubCategory == EStoreSubCategory::Usable_PDA ||
		ItemRow.StoreSubCategory == EStoreSubCategory::Usable_Detector;

	SetBtnVisible(Btn_ChargeItem, bChargeItemVisible);
	SetBtnEnabled(Btn_ChargeItem, bHasBattery);

	// Charge Magazine
	const bool bChargeMagVisible = (ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Magazine);
	SetBtnVisible(Btn_ChargeMagazine, bChargeMagVisible);
	SetBtnEnabled(Btn_ChargeMagazine, bHasAmmo);

	// Repair item
	const bool bRepairVisible =
		ItemRow.StoreCategory == EStoreCategory::storecat_Armor ||
		ItemRow.StoreCategory == EStoreCategory::storecat_Helmet ||
		ItemRow.StoreCategory == EStoreCategory::storecat_Mask ||
		ItemRow.StoreCategory == EStoreCategory::storecat_ASR ||
		ItemRow.StoreCategory == EStoreCategory::storecat_SNP ||
		ItemRow.StoreCategory == EStoreCategory::storecat_SHTG ||
		ItemRow.StoreCategory == EStoreCategory::storecat_HG ||
		ItemRow.StoreCategory == EStoreCategory::storecat_MG ||
		ItemRow.StoreCategory == EStoreCategory::storecat_SMG ||
		ItemRow.StoreCategory == EStoreCategory::storecat_MELEE;

	// доп. фильтр: имеет смысл показывать repair только если предмет реально юзает durability
	const bool bRepairFinal = bRepairVisible && ItemRow.bUseDurability;

	SetBtnVisible(Btn_Repair, bRepairFinal);
	SetBtnEnabled(Btn_Repair, bHasRepairKit);
}

void UItemActionMenuWidget::SetDetachOptions(const TSet<EStoreSubCategory>& AvailableDetachOptions)
{
	SetDetachButtonVisibleFor(EStoreSubCategory::WeaponATTM_Grip, CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Grip));
	SetDetachButtonVisibleFor(EStoreSubCategory::WeaponATTM_Scope, CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Scope));
	SetDetachButtonVisibleFor(EStoreSubCategory::WeaponATTM_Magazine, CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Magazine));
	SetDetachButtonVisibleFor(EStoreSubCategory::WeaponATTM_Laser, CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Laser));
	SetDetachButtonVisibleFor(EStoreSubCategory::WeaponATTM_Flashlight, CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Flashlight));
	SetDetachButtonVisibleFor(EStoreSubCategory::WeaponATTM_Silencer, CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Silencer));
	SetDetachButtonVisibleFor(EStoreSubCategory::WeaponATTM_Module, CachedDetachOptions.Contains(EStoreSubCategory::WeaponATTM_Module));
}

EStoreSubCategory UItemActionMenuWidget::ConsumePendingDetachSubCategory()
{
	const EStoreSubCategory Result = PendingDetachSubCategory;
	PendingDetachSubCategory = EStoreSubCategory::None;
	return Result;
}

void UItemActionMenuWidget::SetOwnerInventory(UInventoryWidget* InOwner)
{
	OwnerInventoryWidget = InOwner;
}

// --- Clicks ---
void UItemActionMenuWidget::ClickEquip()          { Fire(EItemContextAction::Equip); }
void UItemActionMenuWidget::ClickAttach()         { Fire(EItemContextAction::Attach); }
void UItemActionMenuWidget::ClickDetach()         { Fire(EItemContextAction::Detach); }
void UItemActionMenuWidget::ClickDetachGrip()     { QueueDetachAction(EStoreSubCategory::WeaponATTM_Grip); }
void UItemActionMenuWidget::ClickDetachScope()    { QueueDetachAction(EStoreSubCategory::WeaponATTM_Scope); }
void UItemActionMenuWidget::ClickDetachMagazine() { QueueDetachAction(EStoreSubCategory::WeaponATTM_Magazine); }
void UItemActionMenuWidget::ClickDetachLaser()    { QueueDetachAction(EStoreSubCategory::WeaponATTM_Laser); }
void UItemActionMenuWidget::ClickDetachFlashlight() { QueueDetachAction(EStoreSubCategory::WeaponATTM_Flashlight); }
void UItemActionMenuWidget::ClickDetachSilencer() { QueueDetachAction(EStoreSubCategory::WeaponATTM_Silencer); }
void UItemActionMenuWidget::ClickDetachModule()   { QueueDetachAction(EStoreSubCategory::WeaponATTM_Module); }
void UItemActionMenuWidget::ClickUse()            { Fire(EItemContextAction::Use); }
void UItemActionMenuWidget::ClickStudy()          { Fire(EItemContextAction::Study); }
void UItemActionMenuWidget::ClickDrop()           { Fire(EItemContextAction::Drop); }
void UItemActionMenuWidget::ClickChargeItem()     { Fire(EItemContextAction::ChargeItem); }
void UItemActionMenuWidget::ClickChargeMagazine() { Fire(EItemContextAction::ChargeMagazine); }
void UItemActionMenuWidget::ClickRepair()         { Fire(EItemContextAction::Repair); }