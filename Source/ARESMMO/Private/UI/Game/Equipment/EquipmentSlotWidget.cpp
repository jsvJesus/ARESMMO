#include "UI/Game/Equipment/EquipmentSlotWidget.h"
#include "UI/Game/Inventory/Context/ItemDragDropOperation.h"
#include "UI/Game/Inventory/InventoryLayoutWidget.h"
#include "UI/Game/Inventory/InventoryWidget.h"
#include "Items/ItemSizeRules.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Engine/Texture2D.h"
#include "ARESMMO/ARESMMOCharacter.h"

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

UInventoryWidget* UEquipmentSlotWidget::ResolveInventoryWidget() const
{
	if (const UInventoryLayoutWidget* LayoutWidget = GetTypedOuter<UInventoryLayoutWidget>())
	{
		return LayoutWidget->GetActiveInventoryWidget();
	}

	return nullptr;
}

void UEquipmentSlotWidget::SetItem(const FItemBaseRow& ItemRow)
{
	bHasItem = true;
	CurrentItemRow = ItemRow;

	// --- Подгоняем размер слота под размер предмета (клетки * CellSizePx) ---
	if (SlotSizeBox)
	{
		FItemSize Size = ItemRow.GridSize;
		if (Size.Width <= 0 || Size.Height <= 0)
		{
			Size = UItemSizeRules::GetDefaultSize(ItemRow.ItemClass);
		}

		const float W = static_cast<float>(Size.Width)  * CellSizePx;
		const float H = static_cast<float>(Size.Height) * CellSizePx;

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

FReply UEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bHasItem)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bHasItem)
	{
		if (UInventoryWidget* InventoryWidget = ResolveInventoryWidget())
		{
			InventoryWidget->CancelCloseItemActionMenu();
			InventoryWidget->HideItemTooltip();
			InventoryWidget->ShowEquipmentActionMenu(CurrentItemRow, SlotType, InMouseEvent.GetScreenSpacePosition());
		}
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UEquipmentSlotWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	if (!bHasItem)
	{
		OutOperation = nullptr;
		return;
	}

	UItemDragDropOperation* Op = NewObject<UItemDragDropOperation>();
	Op->SourceType = EItemDragSource::Equipment;
	Op->ItemRow    = CurrentItemRow;

	// Размер для возврата в инвентарь — лучше как в инвентаре (rules)
	Op->SizeInCells = CurrentItemRow.GridSize;
	Op->Quantity    = 1;
	Op->FromSlot    = SlotType;

	if (Op->SizeInCells.Width <= 0 || Op->SizeInCells.Height <= 0)
	{
		Op->SizeInCells = UItemSizeRules::GetDefaultSize(CurrentItemRow.ItemClass);
	}

	// Drag visual
	UEquipmentSlotWidget* DragVisual = CreateWidget<UEquipmentSlotWidget>(GetOwningPlayer(), GetClass());
	if (DragVisual)
	{
		DragVisual->SetItem(CurrentItemRow);
		DragVisual->SetRenderOpacity(0.85f);
		USizeBox* DragBox = NewObject<USizeBox>(Op);
		if (DragBox)
		{
			const float Wpx = static_cast<float>(Op->SizeInCells.Width) * CellSizePx;
			const float Hpx = static_cast<float>(Op->SizeInCells.Height) * CellSizePx;
			DragBox->SetWidthOverride(Wpx);
			DragBox->SetHeightOverride(Hpx);
			DragBox->SetContent(DragVisual);
			Op->DefaultDragVisual = DragBox;
		}
		else
		{
			Op->DefaultDragVisual = DragVisual;
		}
		Op->Pivot = EDragPivot::MouseDown;
	}

	OutOperation = Op;

	if (UInventoryWidget* InventoryWidget = ResolveInventoryWidget())
	{
		InventoryWidget->HideItemTooltip();
	}
}

bool UEquipmentSlotWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* Op = Cast<UItemDragDropOperation>(InOperation);
	if (!Op)
		return false;

	AARESMMOCharacter* Char = Cast<AARESMMOCharacter>(GetOwningPlayerPawn());
	if (!Char)
		return false;

	// Инвентарь -> Экипировка
	if (Op->SourceType == EItemDragSource::Inventory)
	{
		return Char->EquipInventoryItemToSlot(Op->ItemRow.InternalName, Op->FromCellX, Op->FromCellY, SlotType);
	}

	// Экипировка -> Экипировка (не обязательно, пока игнорим)
	return (Op->SourceType == EItemDragSource::Equipment && Op->FromSlot == SlotType);
}

void UEquipmentSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (bHasItem)
	{
		if (UInventoryWidget* InventoryWidget = ResolveInventoryWidget())
		{
			InventoryWidget->CancelCloseItemActionMenu();
			InventoryWidget->ShowEquipmentTooltip(CurrentItemRow, 1, InMouseEvent.GetScreenSpacePosition());
		}
	}
}

void UEquipmentSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (UInventoryWidget* InventoryWidget = ResolveInventoryWidget())
	{
		InventoryWidget->HideItemTooltip();
		InventoryWidget->RequestCloseItemActionMenu();
	}
}
