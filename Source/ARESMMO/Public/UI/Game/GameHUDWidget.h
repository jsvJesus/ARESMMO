#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

class UProgressBar;
class UPlayerStatsComponent;
class APawn;
class UImage;
class UTextBlock;

UCLASS()
class ARESMMO_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="ARES|HUD")
	void InitFromPawn(APawn* InPawn);

	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	// ===== Бары здоровья/стамины/радиации =====
	UPROPERTY(meta=(BindWidgetOptional))
	UProgressBar* HealthBar;

	UPROPERTY(meta=(BindWidgetOptional))
	UProgressBar* StaminaBar;

	UPROPERTY(meta=(BindWidgetOptional))
	UProgressBar* RadiationBar;

	// ===== Иконки рисков =====
	// 0% — норм, 100% — всё плохо
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* FoodIcon;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* WaterIcon;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* BleedingIcon;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* BiohazardIcon;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* PsyIcon;

	UPROPERTY(meta=(BindWidgetOptional))
	UImage* ColdIcon;

	// Сводный ambient (электро/огонь/холод/яд)
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* AmbientFireIcon;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* AmbientElectricIcon;

	// Debug Text
	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* FoodPercentText;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* WaterPercentText;

	// Кэш статов
	UPROPERTY()
	UPlayerStatsComponent* Stats = nullptr;

	// Обновление HUD-а
	void UpdateBars();
	void UpdateConditionIcons(float TimeSeconds);
	void UpdateAmbientIcons(float TimeSeconds);

	// Вспомогалки для одной иконки
	void UpdatePrimaryConditionIcon(class UImage* Icon, float Percent, float TimeSeconds);
	void UpdateSecondaryConditionIcon(class UImage* Icon, float Percent, float TimeSeconds);
};