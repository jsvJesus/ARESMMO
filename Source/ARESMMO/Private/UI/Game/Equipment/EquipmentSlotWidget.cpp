#include "UI/Game/Equipment/EquipmentSlotWidget.h"
#include "UI/Game/Inventory/ItemDragDropOperation.h"
#include "UI/Game/Equipment/EquipmentWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Engine/Texture2D.h"
#include "Items/ItemConditionLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "ARESMMO/ARESMMOCharacter.h"

void UEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// При старте слот пустой
	ClearItem();
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
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (!bHasItem)
    {
        return;
    }

    UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(
        UWidgetBlueprintLibrary::CreateDragDropOperation(UItemDragDropOperation::StaticClass()));

    if (!DragOp)
    {
        return;
    }

    DragOp->ItemRow = CurrentItemRow;
    DragOp->ItemSize = CurrentItemRow.GridSize;
    DragOp->bFromEquipment = true;
    DragOp->SourceEquipmentSlot = SlotType;
    DragOp->Pivot = EDragPivot::MouseDown;
	
    UEquipmentSlotWidget* DragVisual = CreateWidget<UEquipmentSlotWidget>(GetOwningPlayer(), GetClass());
    if (DragVisual)
    {
        DragVisual->bHasItem = true;
        DragVisual->CurrentItemRow = CurrentItemRow;
    	
        if (DragVisual->SlotSizeBox)
        {
            const float W = static_cast<float>(CurrentItemRow.GridSize.Width)  * CellSizePx;
            const float H = static_cast<float>(CurrentItemRow.GridSize.Height) * CellSizePx;
            DragVisual->SlotSizeBox->SetWidthOverride(W);
            DragVisual->SlotSizeBox->SetHeightOverride(H);
        }
    	
        if (DragVisual->ItemIcon && ItemIcon)
        {
            DragVisual->ItemIcon->SetBrush(ItemIcon->GetBrush());
            DragVisual->ItemIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    	
        if (DragVisual->NameText)
        {
            DragVisual->NameText->SetText(CurrentItemRow.DisplayName);
            DragVisual->NameText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    	
        if (DragVisual->ConditionText)
        {
            DragVisual->ConditionText->SetText(FText::GetEmpty());
            DragVisual->ConditionText->SetVisibility(ESlateVisibility::Collapsed);
        }

        if (DragVisual->ChargeText)
        {
            DragVisual->ChargeText->SetText(FText::GetEmpty());
            DragVisual->ChargeText->SetVisibility(ESlateVisibility::Collapsed);
        }

        DragVisual->SetIsEnabled(false);

        DragOp->DefaultDragVisual = DragVisual;
    }
    else
    {
        DragOp->DefaultDragVisual = this;
    }

    OutOperation = DragOp;
}

bool UEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation)
{
        const UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(InOperation);

        if (!DragOp || !OwnerEquipment.IsValid())
        {
                return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
        }

        AARESMMOCharacter* Character = OwnerEquipment->GetPreviewCharacter();
        if (!Character)
        {
                return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
        }

        if (!DragOp->bFromEquipment)
        {
                return Character->EquipItemFromInventory(DragOp->ItemRow);
        }

        return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
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

	// Спрятать название
	if (NameText)
		NameText->SetText(FText::GetEmpty());

	// Спрятать прочность
	if (ConditionText)
		ConditionText->SetText(FText::GetEmpty());

	// Спрятать заряд
	if (ChargeText)
		ChargeText->SetText(FText::GetEmpty());
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

	// --- Ставим название ---
	if (NameText)
	{
		NameText->SetText(ItemRow.DisplayName);
	}
	
	// --- Ставим прочность ---
	if (ConditionText)
	{
		const bool bUsesDurability = (ItemRow.bUseRepair && ItemRow.MaxDurability > 0);

		if (bUsesDurability)
		{
			const int32 Max = ItemRow.MaxDurability;
			int32 Current = ItemRow.CurrDurability;
			if (Current <= 0 && ItemRow.DefaultDurability > 0) Current = ItemRow.DefaultDurability;
			Current = FMath::Clamp(Current, 0, Max);

			const int32 PctInt = FMath::RoundToInt((static_cast<float>(Current) / static_cast<float>(Max)) * 100.f);
			ConditionText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PctInt)));

			const EItemConditionState State = UItemConditionLibrary::GetConditionStateFromValues(Current, Max);
			ConditionText->SetColorAndOpacity(UItemConditionLibrary::GetConditionColor(State));
		}
		else
		{
			ConditionText->SetText(FText::GetEmpty());
		}
	}
	
	// --- Ставим заряд ---
	if (ChargeText)
	{
		const bool bUsesCharge = (ItemRow.bUseCharge && ItemRow.MaxCharge > 0);

		if (bUsesCharge)
		{
			const int32 Max = ItemRow.MaxCharge;
			int32 Current = ItemRow.CurrCharge;
			if (Current <= 0 && ItemRow.DefaultCharge > 0) Current = ItemRow.DefaultCharge;
			Current = FMath::Clamp(Current, 0, Max);

			const int32 PctInt = FMath::RoundToInt((static_cast<float>(Current) / static_cast<float>(Max)) * 100.f);
			ChargeText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PctInt)));

			const EItemConditionState State = UItemConditionLibrary::GetConditionStateFromValues(Current, Max);
			ChargeText->SetColorAndOpacity(UItemConditionLibrary::GetConditionColor(State));
		}
		else
		{
			ChargeText->SetText(FText::GetEmpty());
		}
	}
}

void UEquipmentSlotWidget::SetOwnerEquipment(UEquipmentWidget* InOwner)
{
	OwnerEquipment = InOwner;
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
