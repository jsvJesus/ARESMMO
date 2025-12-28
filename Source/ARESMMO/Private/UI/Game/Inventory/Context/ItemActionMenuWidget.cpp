#include "UI/Game/Inventory/Context/ItemActionMenuWidget.h"
#include "UI/Game/Inventory/InventoryWidget.h"
#include "Components/Button.h"

void UItemActionMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Equip)          Btn_Equip->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickEquip);
	if (Btn_Attach)         Btn_Attach->OnClicked.AddDynamic(this, &UItemActionMenuWidget::ClickAttach);
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

void UItemActionMenuWidget::SetOwnerInventory(UInventoryWidget* InOwner)
{
	OwnerInventoryWidget = InOwner;
}

// --- Clicks ---
void UItemActionMenuWidget::ClickEquip()          { Fire(EItemContextAction::Equip); }
void UItemActionMenuWidget::ClickAttach()         { Fire(EItemContextAction::Attach); }
void UItemActionMenuWidget::ClickUse()            { Fire(EItemContextAction::Use); }
void UItemActionMenuWidget::ClickStudy()          { Fire(EItemContextAction::Study); }
void UItemActionMenuWidget::ClickDrop()           { Fire(EItemContextAction::Drop); }
void UItemActionMenuWidget::ClickChargeItem()     { Fire(EItemContextAction::ChargeItem); }
void UItemActionMenuWidget::ClickChargeMagazine() { Fire(EItemContextAction::ChargeMagazine); }
void UItemActionMenuWidget::ClickRepair()         { Fire(EItemContextAction::Repair); }