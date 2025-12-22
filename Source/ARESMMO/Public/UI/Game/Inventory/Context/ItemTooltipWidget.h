#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "ItemTooltipWidget.generated.h"

class UTextBlock;
class UVerticalBox;

UCLASS()
class ARESMMO_API UItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Quantity = сколько в стеке (или 1)
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|Tooltip")
	void SetTooltipData(const FItemBaseRow& ItemRow, int32 Quantity);

protected:
	// --- BindWidget имена должны совпадать с UMG ---
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* ItemDescriptionText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeightText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* CategoryText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* StackText;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* DurabilityText;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* ChargeText;

	// Контейнер для строк эффектов (динамически наполняем)
	UPROPERTY(meta=(BindWidgetOptional))
	UVerticalBox* EffectsBox;

	// Цвета можно потом подогнать
	UPROPERTY(EditDefaultsOnly, Category="ARES|Inventory|Tooltip")
	FLinearColor GoodColor = FLinearColor(0.20f, 1.00f, 0.20f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category="ARES|Inventory|Tooltip")
	FLinearColor BadColor  = FLinearColor(1.00f, 0.25f, 0.25f, 1.0f);

	UPROPERTY(EditDefaultsOnly, Category="ARES|Inventory|Tooltip")
	FLinearColor NeutralColor = FLinearColor(0.90f, 0.90f, 0.90f, 1.0f);

	// Размер текста для строк эффектов (EffectsBox)
	UPROPERTY(EditDefaultsOnly, Category="ARES|Inventory|Tooltip")
	int32 EffectsFontSize = 10;

private:
	void RebuildEffects(const FConsumableEffects& Effects);

	void AddEffectLine(const FString& Label, float Value, bool bGoodWhenPositive, const FString& Suffix = TEXT(""));
	void AddNeutralLine(const FString& Label, const FString& ValueStr, const FLinearColor& Color);

	static FText GetEnumDisplayText(const UEnum* Enum, int64 Value);
};