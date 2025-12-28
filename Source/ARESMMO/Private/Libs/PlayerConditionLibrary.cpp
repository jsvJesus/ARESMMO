#include "Libs/PlayerConditionLibrary.h"

void UPlayerConditionLibrary::GetSecondaryConditionIconParams(
	float Percent,
	FLinearColor& OutColor,
	bool& bOutShouldBlink
)
{
	const float P = FMath::Clamp(Percent, 0.0f, 100.0f);

	// 0..25% — серый, НЕ мигает
	if (P <= 25.0f)
	{
		OutColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f); // серый
		bOutShouldBlink = false;
		return;
	}

	// 25..60% — зелёный, мигает
	if (P <= 60.0f)
	{
		OutColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // зелёный
		bOutShouldBlink = true;
		return;
	}

	// 60..80% — оранжевый, мигает
	if (P <= 80.0f)
	{
		OutColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // оранжевый
		bOutShouldBlink = true;
		return;
	}

	// 80..100% — красный, мигает
	OutColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // красный
	bOutShouldBlink = true;
}

void UPlayerConditionLibrary::GetAmbientConditionIconParams(
	float Percent,
	FLinearColor& OutColor,
	bool& bOutVisible,
	bool& bOutShouldBlink
)
{
	const float P = FMath::Clamp(Percent, 0.0f, 100.0f);

	if (P <= KINDA_SMALL_NUMBER)
	{
		// Иконка выключена
		bOutVisible = false;
		bOutShouldBlink = false;
		OutColor = FLinearColor::Transparent;
		return;
	}

	// Есть урон → иконка видима, всегда красная и мигает
	bOutVisible = true;
	bOutShouldBlink = true;
	OutColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // красный
}

float UPlayerConditionLibrary::GetBlinkAlpha(float TimeSeconds, float Speed)
{
	// sin(t) в диапазоне [-1; 1]
	const float S = FMath::Sin(TimeSeconds * Speed);

	// Перегоняем в [0;1]
	const float Alpha = (S * 0.5f) + 0.5f;

	return Alpha;
}