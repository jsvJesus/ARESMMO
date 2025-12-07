#include "UI/Game/Inventory/InventoryLayoutWidget.h"
#include "UI/Game/Inventory/InventoryWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "InputCoreTypes.h"
#include "ARESMMO/ARESMMOCharacter.h"

void UInventoryLayoutWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// По умолчанию — вкладка "Все" (индекс 0)
	ShowTab(0);
}

void UInventoryLayoutWidget::ShowTab(int32 TabIndex)
{
	if (!InventorySwitcher)
	{
		return;
	}

	if (TabIndex < 0 || TabIndex >= InventorySwitcher->GetNumWidgets())
	{
		return;
	}

	InventorySwitcher->SetActiveWidgetIndex(TabIndex);
}

void UInventoryLayoutWidget::DistributeItems(const TArray<FInventoryItemEntry>& AllItems)
{
	if (Inv_All)    Inv_All->SetAllItems(AllItems);
	if (Inv_Weapon) Inv_Weapon->SetAllItems(AllItems);
	if (Inv_Armor)  Inv_Armor->SetAllItems(AllItems);
	if (Inv_Helmet) Inv_Helmet->SetAllItems(AllItems);
	if (Inv_Medic)  Inv_Medic->SetAllItems(AllItems);
	if (Inv_Food)   Inv_Food->SetAllItems(AllItems);
	if (Inv_Items)  Inv_Items->SetAllItems(AllItems);
	if (Inv_Devices)Inv_Devices->SetAllItems(AllItems);
	if (Inv_Craft)  Inv_Craft->SetAllItems(AllItems);
	if (Inv_Attm)   Inv_Attm->SetAllItems(AllItems);
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

void UInventoryLayoutWidget::InitPreview(class AARESMMOCharacter* Character)
{
	PreviewCharacter = Character;

	if (!Character) return;

	if (UTextureRenderTarget2D* RT = Character->InventoryRenderTarget)
	{
		SetPlayerImage(RT);
	}
}
