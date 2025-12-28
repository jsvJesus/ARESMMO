#include "UI/Game/GameHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/PlayerStatsComponent.h"
#include "Components/TextBlock.h"
#include "Libs/PlayerConditionLibrary.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UGameHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!Stats)
	{
		if (APawn* Pawn = GetOwningPlayerPawn())
		{
			InitFromPawn(Pawn);
		}
	}
}

void UGameHUDWidget::InitFromPawn(APawn* InPawn)
{
	if (!InPawn)
	{
		Stats = nullptr;
		return;
	}

	Stats = InPawn->FindComponentByClass<UPlayerStatsComponent>();
}

void UGameHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!Stats)
	{
		if (APawn* Pawn = GetOwningPlayerPawn())
		{
			InitFromPawn(Pawn);
		}
	}

	UpdateBars();

	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	UpdateConditionIcons(TimeSeconds);
}

/* ================== БАРЫ ================== */
void UGameHUDWidget::UpdateBars()
{
	if (!Stats)
	{
		return;
	}

	const float HealthPct    = FMath::Clamp(Stats->Base.Health    / 100.0f, 0.0f, 1.0f);
	const float StaminaPct   = FMath::Clamp(Stats->Base.Stamina   / 100.0f, 0.0f, 1.0f);
	const float RadiationPct = FMath::Clamp(Stats->Base.Radiation / 100.0f, 0.0f, 1.0f);

	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPct);
	}

	if (StaminaBar)
	{
		StaminaBar->SetPercent(StaminaPct);
	}

	if (RadiationBar)
	{
		RadiationBar->SetPercent(RadiationPct);
	}
}

/* ================== ИКОНКИ УСЛОВИЙ ================== */
void UGameHUDWidget::UpdateConditionIcons(float TimeSeconds)
{
	if (!Stats)
	{
		return;
	}

	// ----- Food / Water: primary-логика (видимость + цвет + мигание) -----
	if (FoodIcon)
	{
		UpdatePrimaryConditionIcon(FoodIcon, Stats->Secondary.Food, TimeSeconds);
	}

	if (WaterIcon)
	{
		UpdatePrimaryConditionIcon(WaterIcon, Stats->Secondary.Water, TimeSeconds);
	}

	auto SetPctText = [&](UTextBlock* Txt, float Percent)
	{
		if (!Txt) return;

		const int32 Pct = FMath::Clamp(FMath::RoundToInt(Percent), 0, 100);
		Txt->SetVisibility(ESlateVisibility::Visible);
		Txt->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), Pct)));

		// По желанию: тот же цвет/мигание, что и иконка
		FLinearColor Color = FLinearColor::White;
		bool bBlink = false;
		UPlayerConditionLibrary::GetSecondaryConditionIconParams(Percent, Color, bBlink);

		Txt->SetColorAndOpacity(Color);

		if (bBlink)
		{
			const float Alpha = UPlayerConditionLibrary::GetBlinkAlpha(TimeSeconds, 8.0f);
			Txt->SetRenderOpacity(Alpha);
		}
		else
		{
			Txt->SetRenderOpacity(1.0f);
		}
	};

	SetPctText(FoodPercentText, Stats->Secondary.Food);
	SetPctText(WaterPercentText, Stats->Secondary.Water);

	// ----- Вторичные статусы: Bleeding / Biohazard / PsyRad -----
	if (BleedingIcon)
	{
		UpdateSecondaryConditionIcon(BleedingIcon, Stats->Secondary.Bleeding, TimeSeconds);
	}

	if (BiohazardIcon)
	{
		const float BioValue    = Stats->Secondary.Biohazard;
		const float PoisonValue = FMath::Max(Stats->Secondary.Poisoning, Stats->Ambient.Poisoning);
		UpdateSecondaryConditionIcon(BiohazardIcon, FMath::Max(BioValue, PoisonValue), TimeSeconds);
	}

	if (PsyIcon)
	{
		UpdateSecondaryConditionIcon(PsyIcon, Stats->Secondary.PsyRad, TimeSeconds);
	}

	// ----- Новые: Cold / Poisoning как вторичные статусы -----
	if (ColdIcon)
	{
		// Cold сидит в Ambient
		UpdateSecondaryConditionIcon(ColdIcon, FMath::Max(Stats->Secondary.Cold, Stats->Ambient.Cold), TimeSeconds);
	}

	// ----- Ambient: Physical / Fire / Electric -> одна из трёх иконок -----
	UpdateAmbientIcons(TimeSeconds);
}

void UGameHUDWidget::UpdateAmbientIcons(float TimeSeconds)
{
	if (!Stats)
	{
		return;
	}

	// Берём только три типа, которые пока хотим отображать
	const float FireValue     = Stats->Ambient.Fire;
	const float ElectricValue = Stats->Ambient.Electro;

	// Cold и Poisoning пока игнорируем для HUD
	// const float ColdValue = Stats->Ambient.Cold;
	// const float PoisoningValue = Stats->Ambient.Poisoning;

	// Максимальное значение среди трёх отображаемых типов
	const float MaxValue = FMath::Max(FireValue, ElectricValue);

	// Если аномального урона нет — прячем все иконки
	if (MaxValue <= KINDA_SMALL_NUMBER)
	{
		if (AmbientFireIcon)
		{
			AmbientFireIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		if (AmbientElectricIcon)
		{
			AmbientElectricIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	enum class EAmbientType : uint8
	{
		None,
		Fire,
		Electric
	};

	EAmbientType ActiveType = EAmbientType::None;

	if (MaxValue == FireValue)
	{
		ActiveType = EAmbientType::Fire;
	}
	else
	{
		ActiveType = EAmbientType::Electric;
	}

	// Цвет и мигание считаем как для вторичных статов
	FLinearColor Color = FLinearColor::White;
	bool bShouldBlink = false;

	UPlayerConditionLibrary::GetSecondaryConditionIconParams(
		MaxValue,
		Color,
		bShouldBlink
	);

	const float Alpha = bShouldBlink
		? UPlayerConditionLibrary::GetBlinkAlpha(TimeSeconds, 8.0f)
		: 1.0f;

	auto SetupIcon = [&](UImage* Icon, bool bActive)
	{
		if (!Icon)
		{
			return;
		}

		if (!bActive)
		{
			Icon->SetVisibility(ESlateVisibility::Hidden);
			return;
		}

		Icon->SetVisibility(ESlateVisibility::Visible);
		Icon->SetColorAndOpacity(Color);
		Icon->SetRenderOpacity(Alpha);
	};

	// Включаем только одну иконку, остальные скрываем
	SetupIcon(AmbientFireIcon,     ActiveType == EAmbientType::Fire);
	SetupIcon(AmbientElectricIcon, ActiveType == EAmbientType::Electric);
}

void UGameHUDWidget::UpdatePrimaryConditionIcon(UImage* Icon, float Percent, float TimeSeconds)
{
	if (!Icon)
	{
		return;
	}

	FLinearColor Color = FLinearColor::White;
	bool bShouldBlink = false;
	
	UPlayerConditionLibrary::GetSecondaryConditionIconParams(Percent, Color, bShouldBlink);

	Icon->SetVisibility(ESlateVisibility::Visible);
	Icon->SetColorAndOpacity(Color);

	if (bShouldBlink)
	{
		const float Alpha = UPlayerConditionLibrary::GetBlinkAlpha(TimeSeconds, 8.0f);
		Icon->SetRenderOpacity(Alpha);
	}
	else
	{
		Icon->SetRenderOpacity(1.0f);
	}
}

void UGameHUDWidget::UpdateSecondaryConditionIcon(UImage* Icon, float Percent, float TimeSeconds)
{
	if (!Icon)
	{
		return;
	}

	FLinearColor Color = FLinearColor::White;
	bool bShouldBlink = false;

	UPlayerConditionLibrary::GetSecondaryConditionIconParams(
		Percent,
		Color,
		bShouldBlink
	);

	// Если стат практически нулевой — иконка может быть просто серой,
	// но мы её не прячем (по желанию можно тут тоже скрывать)
	Icon->SetVisibility(ESlateVisibility::Visible);
	Icon->SetColorAndOpacity(Color);

	if (bShouldBlink)
	{
		const float Alpha = UPlayerConditionLibrary::GetBlinkAlpha(TimeSeconds, 8.0f);
		Icon->SetRenderOpacity(Alpha);
	}
	else
	{
		Icon->SetRenderOpacity(1.0f);
	}
}