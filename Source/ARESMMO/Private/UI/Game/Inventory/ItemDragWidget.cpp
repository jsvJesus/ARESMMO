#include "UI/Game/Inventory/ItemDragWidget.h"
#include "UI/Game/Inventory/ItemDragDropOperation.h"
#include "UI/Game/Inventory/ItemDragDropWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UItemDragWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BackgroundImage) BackgroundImage->SetVisibility(ESlateVisibility::Visible);
	if (IconImage)       IconImage->SetVisibility(ESlateVisibility::Visible);
	if (NameText)        NameText->SetVisibility(ESlateVisibility::Visible);
	if (WeightText)      WeightText->SetVisibility(ESlateVisibility::Visible);
	if (ConditionText)   ConditionText->SetVisibility(ESlateVisibility::Visible);
	if (ChargeText)      ChargeText->SetVisibility(ESlateVisibility::Visible);
	if (StackText)       StackText->SetVisibility(ESlateVisibility::Visible);
}

FReply UItemDragWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UItemDragWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(
		UWidgetBlueprintLibrary::CreateDragDropOperation(UItemDragDropOperation::StaticClass()));

	if (!DragOp)
	{
		return;
	}

	// Данные (как у тебя было раньше в ItemSlotWidget) :contentReference[oaicite:2]{index=2}
	DragOp->ItemRow = CurrentItemRow;
	DragOp->ItemSize = CurrentItemSize;
	DragOp->SourceCellX = CurrentCellX;
	DragOp->SourceCellY = CurrentCellY;
	DragOp->bFromEquipment = bFromEquipment;
	DragOp->SourceEquipmentSlot = SourceEquipmentSlot;
	DragOp->Pivot = EDragPivot::MouseDown;

	// Визуал под курсором
	UUserWidget* DragVisual = nullptr;

	if (DragVisualClass)
	{
		if (UItemDragDropWidget* VisualWidget = CreateWidget<UItemDragDropWidget>(GetOwningPlayer(), DragVisualClass))
		{
			VisualWidget->InitDragVisual(CurrentItemRow, CurrentItemSize, CellSizePx);
			VisualWidget->SetDesiredSizeInViewport(FVector2D(CurrentItemSize.Width * CellSizePx, CurrentItemSize.Height * CellSizePx));
			DragVisual = VisualWidget;
		}
	}

	DragOp->DefaultDragVisual = DragVisual ? DragVisual : this;
	OutOperation = DragOp;
}