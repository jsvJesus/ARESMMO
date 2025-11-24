#include "Items/ItemConditionLibrary.h"

EItemConditionState UItemConditionLibrary::GetConditionStateFromValues(int32 CurrentValue, int32 MaxValue)
{
	if (MaxValue <= 0)
	{
		return EItemConditionState::Broken;
	}

	const float Percent = (static_cast<float>(CurrentValue) / static_cast<float>(MaxValue)) * 100.0f;

	if (Percent >= 100.0f) return EItemConditionState::Perfect;
	if (Percent >= 75.0f)  return EItemConditionState::Good;
	if (Percent >= 50.0f)  return EItemConditionState::Medium;
	if (Percent >= 25.0f)  return EItemConditionState::Low;
	if (Percent >= 2.0f)   return EItemConditionState::Bad;

	return EItemConditionState::Broken;
}

FLinearColor UItemConditionLibrary::GetConditionColor(EItemConditionState State)
{
	switch (State)
	{
	case EItemConditionState::Perfect: return FLinearColor(0.0f, 0.4f, 1.0f);  // Синий
	case EItemConditionState::Good:    return FLinearColor::Green;             // Зеленый
	case EItemConditionState::Medium:  return FLinearColor::Yellow;            // Желтый
	case EItemConditionState::Low:     return FLinearColor(1.0f, 0.65f, 0.0f); // Оранжевый
	case EItemConditionState::Bad:     return FLinearColor::Red;               // Красный
	case EItemConditionState::Broken:  return FLinearColor::Black;             // Черный
	default:                           return FLinearColor::White;
	}
}