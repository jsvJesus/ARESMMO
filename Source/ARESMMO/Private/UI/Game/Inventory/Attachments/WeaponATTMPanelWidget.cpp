#include "UI/Game/Inventory/Attachments/WeaponATTMPanelWidget.h"
#include "UI/Game/Inventory/Attachments/WeaponATTMSlotWidget.h"
#include "Weapons/WeaponBase.h"

void UWeaponATTMPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindSlot(Slot_Scope,      EStoreSubCategory::WeaponATTM_Scope);
	BindSlot(Slot_Grip,       EStoreSubCategory::WeaponATTM_Grip);
	BindSlot(Slot_Magazine,   EStoreSubCategory::WeaponATTM_Magazine);
	BindSlot(Slot_Laser,      EStoreSubCategory::WeaponATTM_Laser);
	BindSlot(Slot_Flashlight, EStoreSubCategory::WeaponATTM_Flashlight);
	BindSlot(Slot_Silencer,   EStoreSubCategory::WeaponATTM_Silencer);
	BindSlot(Slot_Module,     EStoreSubCategory::WeaponATTM_Module);

	Refresh();
}

void UWeaponATTMPanelWidget::SetWeapon(AWeaponBase* InWeapon)
{
	Weapon = InWeapon;
	Refresh();
}

void UWeaponATTMPanelWidget::Refresh()
{
	UpdateSlot(Slot_Scope,      EStoreSubCategory::WeaponATTM_Scope);
	UpdateSlot(Slot_Grip,       EStoreSubCategory::WeaponATTM_Grip);
	UpdateSlot(Slot_Magazine,   EStoreSubCategory::WeaponATTM_Magazine);
	UpdateSlot(Slot_Laser,      EStoreSubCategory::WeaponATTM_Laser);
	UpdateSlot(Slot_Flashlight, EStoreSubCategory::WeaponATTM_Flashlight);
	UpdateSlot(Slot_Silencer,   EStoreSubCategory::WeaponATTM_Silencer);
	UpdateSlot(Slot_Module,     EStoreSubCategory::WeaponATTM_Module);
}

void UWeaponATTMPanelWidget::BindSlot(UWeaponATTMSlotWidget* SlotWidget, EStoreSubCategory SubCategory)
{
	if (!SlotWidget) return;

	SlotWidget->SetSlot(SubCategory);
	SlotWidget->OnDetachClicked.AddDynamic(this, &UWeaponATTMPanelWidget::HandleDetachClicked);
}

void UWeaponATTMPanelWidget::UpdateSlot(UWeaponATTMSlotWidget* SlotWidget, EStoreSubCategory SubCategory) const
{
	if (!SlotWidget) return;

	if (!Weapon.IsValid())
	{
		SlotWidget->SetSlot(SubCategory);
		return;
	}

	const FAttachedWeaponATTM* Found = Weapon->AttachedATTM.Find(SubCategory);
	if (Found && Found->bValid)
	{
		SlotWidget->SetItem(Found->ItemRow);
	}
	else
	{
		SlotWidget->SetSlot(SubCategory);
	}
}

void UWeaponATTMPanelWidget::HandleDetachClicked(EStoreSubCategory SlotSubCategory)
{
	OnDetachRequested.Broadcast(SlotSubCategory);
}