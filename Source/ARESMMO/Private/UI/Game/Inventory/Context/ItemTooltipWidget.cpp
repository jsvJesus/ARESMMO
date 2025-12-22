#include "UI/Game/Inventory/Context/ItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

FText UItemTooltipWidget::GetEnumDisplayText(const UEnum* Enum, int64 Value)
{
	if (!Enum)
	{
		return FText::GetEmpty();
	}
	return Enum->GetDisplayNameTextByValue(Value);
}

void UItemTooltipWidget::SetTooltipData(const FItemBaseRow& ItemRow, int32 Quantity)
{
	if (ItemNameText)        ItemNameText->SetText(ItemRow.DisplayName);
	if (ItemDescriptionText) ItemDescriptionText->SetText(ItemRow.Description);

	// Категория (EStoreCategory)
	{
		const UEnum* CatEnum = StaticEnum<EStoreCategory>();
		const FText CatText = GetEnumDisplayText(CatEnum, (int64)ItemRow.StoreCategory);
		if (CategoryText)
		{
			CategoryText->SetText(CatText);
		}
	}

	// Стек
	if (StackText)
	{
		if (ItemRow.bUseStack)
		{
			const int32 MaxStack = ItemRow.GetEffectiveMaxStackSize();
			StackText->SetText(FText::FromString(FString::Printf(TEXT("x%d"), FMath::Max(1, Quantity))));
		}
		//else
		//{
			//StackText->SetText(FText::FromString(TEXT("x1")));
		//}
	}

	// Вес (итоговый: unit * quantity)
	if (WeightText)
	{
		const float UnitKg = ItemRow.GetUnitWeightKg();
		const float TotalKg = UnitKg * (float)FMath::Max(1, Quantity);
		WeightText->SetText(FText::FromString(FString::Printf(TEXT("%.2f kg"), TotalKg)));
	}

	// Прочность (используем DefaultDurability = “текущее” значение в инстансе)
	if (DurabilityText)
	{
		if (ItemRow.bUseDurability && ItemRow.MaxDurability > 0)
		{
			const int32 Cur = FMath::Clamp(ItemRow.DefaultDurability, 0, ItemRow.MaxDurability);
			const int32 Max = ItemRow.MaxDurability;
			const int32 Pct = FMath::RoundToInt((float)Cur / (float)Max * 100.f);

			DurabilityText->SetText(FText::FromString(FString::Printf(TEXT("Durability: %d%% (%d/%d)"), Pct, Cur, Max)));
			DurabilityText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			DurabilityText->SetText(FText::GetEmpty());
			DurabilityText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Заряд
	if (ChargeText)
	{
		if (ItemRow.bUseCharge && ItemRow.MaxCharge > 0)
		{
			const int32 Cur = FMath::Clamp(ItemRow.DefaultCharge, 0, ItemRow.MaxCharge);
			const int32 Max = ItemRow.MaxCharge;
			const int32 Pct = FMath::RoundToInt((float)Cur / (float)Max * 100.f);

			ChargeText->SetText(FText::FromString(FString::Printf(TEXT("Charge: %d%% (%d/%d)"), Pct, Cur, Max)));
			ChargeText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ChargeText->SetText(FText::GetEmpty());
			ChargeText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	RebuildEffects(ItemRow.ConsumableEffects);
}

void UItemTooltipWidget::AddNeutralLine(const FString& Label, const FString& ValueStr, const FLinearColor& Color)
{
	if (!EffectsBox || !WidgetTree) return;

	UTextBlock* Line = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (!Line) return;

	Line->SetText(FText::FromString(FString::Printf(TEXT("%s: %s"), *Label, *ValueStr)));
	Line->SetColorAndOpacity(FSlateColor(Color));

	// Наследуем шрифт от основных TextBlock'ов (в приоритете WeightText)
	FSlateFontInfo BaseFont;

	if (WeightText)
	{
		BaseFont = WeightText->GetFont();
	}
	else if (ItemDescriptionText)
	{
		BaseFont = ItemDescriptionText->GetFont();
	}
	else if (ItemNameText)
	{
		BaseFont = ItemNameText->GetFont();
	}
	else
	{
		BaseFont = Line->GetFont(); // fallback
	}

	BaseFont.Size = EffectsFontSize; // 10
	Line->SetFont(BaseFont);

	EffectsBox->AddChild(Line);
}

void UItemTooltipWidget::AddEffectLine(const FString& Label, float Value, bool bGoodWhenPositive, const FString& Suffix)
{
	if (FMath::IsNearlyZero(Value))
	{
		return;
	}

	const bool bPositive = (Value > 0.f);
	const bool bGood = (bPositive == bGoodWhenPositive);

	const FLinearColor UseColor = bGood ? GoodColor : BadColor;

	FString ValueStr;
	if (Suffix.Len() > 0)
	{
		ValueStr = FString::Printf(TEXT("%+.1f%s"), Value, *Suffix);
	}
	else
	{
		ValueStr = FString::Printf(TEXT("%+.1f"), Value);
	}

	AddNeutralLine(Label, ValueStr, UseColor);
}

void UItemTooltipWidget::RebuildEffects(const FConsumableEffects& Effects)
{
	if (!EffectsBox)
	{
		return;
	}

	EffectsBox->ClearChildren();

	// Если вообще нет эффектов — можно оставить пусто
	const bool bHasAny =
		!FMath::IsNearlyZero(Effects.HealthDelta) ||
		!FMath::IsNearlyZero(Effects.StaminaDelta) ||
		!FMath::IsNearlyZero(Effects.FoodDelta) ||
		!FMath::IsNearlyZero(Effects.WaterDelta) ||
		!FMath::IsNearlyZero(Effects.RadiationDelta) ||
		!FMath::IsNearlyZero(Effects.BiohazardDelta) ||
		!FMath::IsNearlyZero(Effects.PsyRadDelta) ||
		!FMath::IsNearlyZero(Effects.PoisoningDelta) ||
		!FMath::IsNearlyZero(Effects.ColdDelta) ||
		!FMath::IsNearlyZero(Effects.BadEffectChance) ||
		!FMath::IsNearlyZero(Effects.BadEffectPower);

	if (!bHasAny)
	{
		return;
	}

	// Health / Stamina: плюс = хорошо
	AddEffectLine(TEXT("Health"),  Effects.HealthDelta,  true);
	AddEffectLine(TEXT("Stamina"), Effects.StaminaDelta, true);

	// Остальные: минус = хорошо (меньше голода/жажды/радиации и т.п.)
	AddEffectLine(TEXT("Food"),       Effects.FoodDelta,       false);
	AddEffectLine(TEXT("Water"),      Effects.WaterDelta,      false);
	AddEffectLine(TEXT("Radiation"),  Effects.RadiationDelta,  false);
	AddEffectLine(TEXT("Biohazard"),  Effects.BiohazardDelta,  false);
	AddEffectLine(TEXT("PsyRad"),     Effects.PsyRadDelta,     false);
	AddEffectLine(TEXT("Poisoning"),  Effects.PoisoningDelta,  false);
	AddEffectLine(TEXT("Cold"),       Effects.ColdDelta,       false);

	// Плохие эффекты (нейтрально/желтым)
	if (!FMath::IsNearlyZero(Effects.BadEffectChance))
	{
		AddNeutralLine(TEXT("Bad effect chance"), FString::Printf(TEXT("%.0f%%"), Effects.BadEffectChance),
			FLinearColor(1.f, 0.85f, 0.25f, 1.f));
	}
	if (!FMath::IsNearlyZero(Effects.BadEffectPower))
	{
		AddNeutralLine(TEXT("Bad effect power"), FString::Printf(TEXT("%.1f"), Effects.BadEffectPower),
			FLinearColor(1.f, 0.65f, 0.25f, 1.f));
	}
}