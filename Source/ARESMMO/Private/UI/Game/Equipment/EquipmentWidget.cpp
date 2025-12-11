#include "UI/Game/Equipment/EquipmentWidget.h"
#include "UI/Game/Equipment/EquipmentSlotWidget.h"

void UEquipmentWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SlotMap.Empty();

	auto Reg = [this](UEquipmentSlotWidget* Widget, EEquipmentSlotType Type)
	{
		if (Widget)
		{
			Widget->SlotType = Type;      // на всякий случай продублируем
			SlotMap.Add(Type, Widget);

			// подписываемся на даблклик по этому слоту
			Widget->OnSlotDoubleClicked.AddDynamic(this, &UEquipmentWidget::HandleSlotDoubleClicked);
		}
	};

	// Hero / одежда
	Reg(Slot_Helmet,   EEquipmentSlotType::EquipmentSlotHelmet);
	Reg(Slot_Body,    EEquipmentSlotType::EquipmentSlotBody);
	Reg(Slot_Mask,     EEquipmentSlotType::EquipmentSlotMask);

	Reg(Slot_Armor,    EEquipmentSlotType::EquipmentSlotArmor);
	Reg(Slot_Legs,    EEquipmentSlotType::EquipmentSlotLegs);
	Reg(Slot_Backpack, EEquipmentSlotType::EquipmentSlotBackpack);

	// Оружие
	Reg(Slot_Weapon1,  EEquipmentSlotType::EquipmentSlotWeapon1);
	Reg(Slot_Weapon2,  EEquipmentSlotType::EquipmentSlotWeapon2);
	Reg(Slot_Pistol,   EEquipmentSlotType::EquipmentSlotPistol);
	Reg(Slot_Knife,    EEquipmentSlotType::EquipmentSlotKnife);

	// Девайсы
	Reg(Slot_Device1,  EEquipmentSlotType::EquipmentSlotDevice1);
	Reg(Slot_Device2,  EEquipmentSlotType::EquipmentSlotDevice2);

	ClearAllSlots();
}

void UEquipmentWidget::ClearAllSlots()
{
	for (auto& Pair : SlotMap)
	{
		if (Pair.Value)
		{
			Pair.Value->ClearItem();
		}
	}
}

void UEquipmentWidget::SetEquipment(const TMap<EEquipmentSlotType, FItemBaseRow>& InEquipment)
{
	ClearAllSlots();

	for (const auto& Pair : InEquipment)
	{
		if (UEquipmentSlotWidget** FoundSlot = SlotMap.Find(Pair.Key))
		{
			if (*FoundSlot)
			{
				(*FoundSlot)->SetItem(Pair.Value);
			}
		}
	}
}

void UEquipmentWidget::HandleSlotDoubleClicked(UEquipmentSlotWidget* ClickedSlot)
{
	if (!ClickedSlot || !ClickedSlot->bHasItem)
	{
		return;
	}

	// просто пробрасываем наружу тип слота
	OnUnequipRequested.Broadcast(ClickedSlot->SlotType);
}