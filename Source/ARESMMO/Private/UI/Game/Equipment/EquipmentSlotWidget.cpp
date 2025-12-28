#include "UI/Game/Equipment/EquipmentSlotWidget.h"
#include "UI/Game/Inventory/Context/ItemDragDropOperation.h"
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
	UEquipmentSlotWidget* DragVisual = CreateWidget<UEquipmentSlotWidget>(GetWorld(), GetClass());
	if (DragVisual)
	{
		DragVisual->SetItem(CurrentItemRow);
		DragVisual->SetRenderOpacity(0.85f);
		Op->DefaultDragVisual = DragVisual;
		Op->Pivot = EDragPivot::MouseDown;
	}

	OutOperation = Op;
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