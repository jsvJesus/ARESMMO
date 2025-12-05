#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PlayerConditionLibrary.generated.h"

/**
 * Логика цветов и мигания иконок для статов персонажа.
 * Работает с процентами 0..100.
 */
UCLASS()
class ARESMMO_API UPlayerConditionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Второстепенные статы (Biohazard, PsyRad, Bleeding, Weight, Food, Water):
	 *
	 * - 0%        -> Серый, не мигает
	 * - 0 < x <=25 -> Зелёный, мигает
	 * - 25 < x <=60 -> Жёлтый, мигает
	 * - 60 < x <=80 -> Оранжевый, мигает
	 * - 80 < x <=100 -> Красный, мигает
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|UI")
	static void GetSecondaryConditionIconParams(
		float Percent,
		FLinearColor& OutColor,
		bool& bOutShouldBlink
	);

	/**
	 * Ambient статы (Electro, Fire, Physical, Cold, Poisoning):
	 *
	 * - Показываем картинку, только если Percent > 0
	 * - Цвет всегда Красный
	 * - Всегда мигает, когда активен
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|UI")
	static void GetAmbientConditionIconParams(
		float Percent,
		FLinearColor& OutColor,
		bool& bOutVisible,
		bool& bOutShouldBlink
	);

	/**
	 * Вспомогательная функция для мигания.
	 * TimeSeconds — обычно GetWorld()->GetTimeSeconds()
	 *
	 * Возвращает Alpha в диапазоне 0..1.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="ARES|UI")
	static float GetBlinkAlpha(float TimeSeconds, float Speed = 8.0f);
};