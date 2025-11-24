#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemConditionLibrary.generated.h"

UENUM(BlueprintType)
enum class EItemConditionState : uint8
{
	Broken   UMETA(DisplayName="0-1% (Black)"),
	Bad      UMETA(DisplayName="2-24% (Red)"),
	Low      UMETA(DisplayName="25-49% (Orange)"),
	Medium   UMETA(DisplayName="50-74% (Yellow)"),
	Good     UMETA(DisplayName="75-99% (Green)"),
	Perfect  UMETA(DisplayName="100% (Blue)")
};

UCLASS()
class ARESMMO_API UItemConditionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** Расчёт состояния по текущему и максимальному значению (прочность или заряд) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	static EItemConditionState GetConditionStateFromValues(int32 CurrentValue, int32 MaxValue);

	/** Цвет для состояния (UI) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|Item")
	static FLinearColor GetConditionColor(EItemConditionState State);
};