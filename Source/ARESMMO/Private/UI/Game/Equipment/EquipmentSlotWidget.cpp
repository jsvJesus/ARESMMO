#include "UI/Game/Equipment/EquipmentSlotWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Engine/Texture2D.h"

void UEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// При старте слот пустой
	ClearItem();
}

void UEquipmentSlotWidget::ClearItem()
{
	bHasItem = false;
	CurrentItemRow = FItemBaseRow(); // обнулить

	// Спрятать иконку
	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(nullptr);
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	// Сбросить размеры
	if (SlotSizeBox)
	{
		SlotSizeBox->ClearWidthOverride();
		SlotSizeBox->ClearHeightOverride();
	}
}

void UEquipmentSlotWidget::SetItem(const FItemBaseRow& ItemRow)
{
	bHasItem = true;
	CurrentItemRow = ItemRow;

	// --- Подгоняем размер слота под размер предмета (клетки * CellSizePx) ---
	if (SlotSizeBox)
	{
		const float W = static_cast<float>(ItemRow.GridSize.Width)  * CellSizePx;
		const float H = static_cast<float>(ItemRow.GridSize.Height) * CellSizePx;

		SlotSizeBox->SetWidthOverride(W);
		SlotSizeBox->SetHeightOverride(H);
	}

	// --- Ставим иконку ---
	if (ItemIcon)
	{
		UTexture2D* IconTex = ItemRow.Icon.LoadSynchronous();
		if (IconTex)
		{
			ItemIcon->SetBrushFromTexture(IconTex, true);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemIcon->SetBrushFromTexture(nullptr);
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

FReply UEquipmentSlotWidget::NativeOnMouseButtonDoubleClick(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	// Нас интересует ЛКМ и только если в слоте есть предмет
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bHasItem)
	{
		OnSlotDoubleClicked.Broadcast(this);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}