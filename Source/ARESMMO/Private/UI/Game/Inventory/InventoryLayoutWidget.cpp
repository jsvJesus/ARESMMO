#include "UI/Game/Inventory/InventoryLayoutWidget.h"
#include "UI/Game/Inventory/InventoryWidget.h"
#include "UI/Game/Equipment/EquipmentWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/TextureRenderTarget2D.h"
#include "InputCoreTypes.h"
#include "ARESMMO/ARESMMOCharacter.h"

void UInventoryLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// По умолчанию — вкладка "Все" (индекс 0)
	ShowTab(0);

	// Слушаем даблклики по слотам экипировки
	if (EquipmentPanel)
	{
		EquipmentPanel->OnUnequipRequested.AddDynamic(this, &UInventoryLayoutWidget::HandleUnequipRequested);
	}
	
	// --- Подписка на даблклик по предметам в инвентаре ---
	auto BindInventory = [this](UInventoryWidget* InvWidget)
	{
		if (InvWidget)
		{
			InvWidget->OnItemEquipRequested.AddDynamic(
				this,
				&UInventoryLayoutWidget::HandleInventoryItemEquipRequested
			);
		}
	};

	BindInventory(Inv_All);
	BindInventory(Inv_Weapon);
	BindInventory(Inv_Armor);
	BindInventory(Inv_Helmet);
	BindInventory(Inv_Medic);
	BindInventory(Inv_Food);
	BindInventory(Inv_Items);
	BindInventory(Inv_Devices);
	BindInventory(Inv_Craft);
	BindInventory(Inv_Attm);

	RefreshWeightText();
}

void UInventoryLayoutWidget::SetTemporaryCarryWeightBonusKg(float BonusKg)
{
	TemporaryCarryWeightBonusKg = BonusKg;
	RefreshWeightText();
}

void UInventoryLayoutWidget::RefreshWeightText()
{
	const float CurrentKg =
		CalcInventoryWeightKg(CachedInventoryItems) +
		CalcEquipmentWeightKg(CachedEquipment);

	const float MaxKg =
		BaseCarryWeightKg +
		CalcCarryBonusFromEquipmentKg(CachedEquipment) +
		TemporaryCarryWeightBonusKg;

	if (TotalWeightText)
	{
		// Total weight: 36.7 (max 70kg)
		const FString S = FString::Printf(TEXT("Total weight: %.1f (max %.0fkg)"), CurrentKg, MaxKg);
		TotalWeightText->SetText(FText::FromString(S));
	}
}

void UInventoryLayoutWidget::ShowTab(int32 TabIndex)
{
	if (!InventorySwitcher)
	{
		return;
	}

	// Проверка индекса
	if (TabIndex < 0 || TabIndex >= InventorySwitcher->GetNumWidgets())
	{
		return;
	}

	// Переключаем вкладку в WidgetSwitcher
	InventorySwitcher->SetActiveWidgetIndex(TabIndex);

	// Запоминаем активную вкладку
	CurrentTabIndex = TabIndex;

	// Обновляем визуал всех кнопок
	UpdateTabButtons();
}

void UInventoryLayoutWidget::UpdateTabButtons()
{
	// Вспомогательная функция — клонируем базовый стиль и подставляем свою картинку
	auto ApplyStyleWithIcon = [this](UButton* Btn, int32 Index, UTexture2D* Icon)
	{
		if (!Btn)
		{
			return;
		}

		const bool bIsActive = (Index == CurrentTabIndex);

		// Берём базовый стиль
		FButtonStyle StyleToApply = bIsActive ? TabActiveBaseStyle : TabNormalBaseStyle;

		// Подставляем нужную иконку во все состояния
		if (Icon)
		{
			auto SetBrushIcon = [Icon](FSlateBrush& Brush)
			{
				Brush.SetResourceObject(Icon);
				Brush.ImageSize = FVector2D(
					static_cast<float>(Icon->GetSizeX()),
					static_cast<float>(Icon->GetSizeY())
				);
			};

			SetBrushIcon(StyleToApply.Normal);
			SetBrushIcon(StyleToApply.Hovered);
			SetBrushIcon(StyleToApply.Pressed);
		}

		Btn->SetStyle(StyleToApply);
	};

	ApplyStyleWithIcon(Btn_All,     0, Icon_All);
	ApplyStyleWithIcon(Btn_Weapon,  1, Icon_Weapon);
	ApplyStyleWithIcon(Btn_Armor,   2, Icon_Armor);
	ApplyStyleWithIcon(Btn_Helmet,  3, Icon_Helmet);
	ApplyStyleWithIcon(Btn_Medic,   4, Icon_Medic);
	ApplyStyleWithIcon(Btn_Food,    5, Icon_Food);
	ApplyStyleWithIcon(Btn_Items,   6, Icon_Items);
	ApplyStyleWithIcon(Btn_Devices, 7, Icon_Devices);
	ApplyStyleWithIcon(Btn_Craft,   8, Icon_Craft);
	ApplyStyleWithIcon(Btn_Attm,    9, Icon_Attm);
}

void UInventoryLayoutWidget::DistributeItems(const TArray<FInventoryItemEntry>& AllItems)
{
	if (Inv_All)    Inv_All->SetAllItems(AllItems); // All
	if (Inv_Weapon) Inv_Weapon->SetAllItems(AllItems); // Weapon
	if (Inv_Armor)  Inv_Armor->SetAllItems(AllItems); // Armor
	if (Inv_Helmet) Inv_Helmet->SetAllItems(AllItems); // Cloth
	if (Inv_Medic)  Inv_Medic->SetAllItems(AllItems); // Medicine
	if (Inv_Food)   Inv_Food->SetAllItems(AllItems); // Food
	if (Inv_Items)  Inv_Items->SetAllItems(AllItems); // Items
	if (Inv_Devices)Inv_Devices->SetAllItems(AllItems); // Device
	if (Inv_Craft)  Inv_Craft->SetAllItems(AllItems); // Craft
	if (Inv_Attm)   Inv_Attm->SetAllItems(AllItems); // Attm

	CachedInventoryItems = AllItems;
	RefreshWeightText();
}

void UInventoryLayoutWidget::SetEquipment(const TMap<EEquipmentSlotType, FItemBaseRow>& Equipment)
{
	if (EquipmentPanel)
	{
		EquipmentPanel->SetEquipment(Equipment);
	}

	CachedEquipment = Equipment;
	RefreshWeightText();
}

void UInventoryLayoutWidget::SetPlayerImage(UTextureRenderTarget2D* RenderTarget)
{
	if (!PlayerRef || !RenderTarget)
	{
		return;
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(RenderTarget);
	Brush.ImageSize = FVector2D(RenderTarget->SizeX, RenderTarget->SizeY);

	PlayerRef->SetBrush(Brush);
}

UInventoryWidget* UInventoryLayoutWidget::GetActiveInventoryWidget() const
{
	if (InventorySwitcher)
	{
		if (UWidget* ActiveWidget = InventorySwitcher->GetActiveWidget())
		{
			if (UInventoryWidget* ActiveInventory = Cast<UInventoryWidget>(ActiveWidget))
			{
				return ActiveInventory;
			}
		}
	}

	if (Inv_All) return Inv_All;
	if (Inv_Weapon) return Inv_Weapon;
	if (Inv_Armor) return Inv_Armor;
	if (Inv_Helmet) return Inv_Helmet;
	if (Inv_Medic) return Inv_Medic;
	if (Inv_Food) return Inv_Food;
	if (Inv_Items) return Inv_Items;
	if (Inv_Devices) return Inv_Devices;
	if (Inv_Craft) return Inv_Craft;
	if (Inv_Attm) return Inv_Attm;

	return nullptr;
}

FReply UInventoryLayoutWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Если нажали ЛКМ по области с PlayerRef – начинаем крутить
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (PlayerRef && PlayerRef->IsHovered())
		{
			bIsRotatingPreview = true;
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UInventoryLayoutWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsRotatingPreview = false;
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UInventoryLayoutWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsRotatingPreview && PreviewCharacter.IsValid())
	{
		FVector2D Delta = InMouseEvent.GetCursorDelta();
		PreviewCharacter->AddInventoryPreviewYaw(Delta.X);
		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

float UInventoryLayoutWidget::CalcInventoryWeightKg(const TArray<FInventoryItemEntry>& Items) const
{
	float Total = 0.f;

	for (const FInventoryItemEntry& Entry : Items)
	{
		const FItemBaseRow& Row = Entry.ItemRow;

		if (!Row.bUseWeight)
		{
			continue;
		}

		const float UnitW = FMath::Max(0.f, Row.Weight);
		const int32 Count = Row.bUseStack ? FMath::Max(1, Entry.Quantity) : 1;

		Total += UnitW * static_cast<float>(Count);
	}

	return Total;
}

float UInventoryLayoutWidget::CalcEquipmentWeightKg(const TMap<EEquipmentSlotType, FItemBaseRow>& Equipment) const
{
	float Total = 0.f;

	for (const auto& Pair : Equipment)
	{
		const FItemBaseRow& Row = Pair.Value;

		if (!Row.bUseWeight)
		{
			continue;
		}

		Total += FMath::Max(0.f, Row.Weight);
	}

	return Total;
}

float UInventoryLayoutWidget::CalcCarryBonusFromEquipmentKg(
	const TMap<EEquipmentSlotType, FItemBaseRow>& Equipment) const
{
	// Бонус даёт только рюкзак в EquipmentSlotBackpack
	if (const FItemBaseRow* BackpackRow = Equipment.Find(EEquipmentSlotType::EquipmentSlotBackpack))
	{
		return BackpackRow->CarryWeightBonus;
	}
	return 0.f;
}

void UInventoryLayoutWidget::InitPreview(class AARESMMOCharacter* Character)
{
	PreviewCharacter = Character;

	if (!Character) return;

	if (UTextureRenderTarget2D* RT = Character->InventoryRenderTarget)
	{
		SetPlayerImage(RT);
	}
}

void UInventoryLayoutWidget::HandleUnequipRequested(EEquipmentSlotType SlotType)
{
	if (!PreviewCharacter.IsValid())
	{
		return;
	}

	if (AARESMMOCharacter* Char = PreviewCharacter.Get())
	{
		Char->UnequipSlot(SlotType);
	}
}

void UInventoryLayoutWidget::HandleInventoryItemEquipRequested(const FItemBaseRow& ItemRow)
{
	if (!PreviewCharacter.IsValid())
	{
		return;
	}

	if (AARESMMOCharacter* Char = PreviewCharacter.Get())
	{
		Char->EquipItemFromInventory(ItemRow);
	}
}