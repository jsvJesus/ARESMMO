#include "UI/Game/Equipment/EquipmentSlotWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Items/ItemConditionLibrary.h"

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