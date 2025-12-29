#include "Components/PlayerStatsComponent.h"
#include "GameFramework/Character.h"
#include "Items/MasterItemData.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerStatsComponent::TickComponent(
	float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickGoodBadStats(DeltaTime);
	TickAmbient(DeltaTime);
	TickBleeding(DeltaTime);
	TickWeight(DeltaTime);
	TickRadiation(DeltaTime);
	TickPsyHazards(DeltaTime);
	TickBioHazards(DeltaTime);

	RestoreStamina(DeltaTime);
	ClampAll();
}

/* ====== Публичное API ====== */
void UPlayerStatsComponent::ApplyHealthDamage(float Damage)
{
	if (Damage <= 0.f)
		return;

	Base.Health = FMath::Clamp(Base.Health - Damage, 0.f, 100.f);
}

void UPlayerStatsComponent::ApplyConsumable(const FItemBaseRow& ItemRow)
{
	// Основной выбор — по ItemClass
	switch (ItemRow.ItemClass)
	{
	case EItemClass::Food:
		ApplyFoodEffects(ItemRow);
		break;

	case EItemClass::Water:
		ApplyWaterEffects(ItemRow);
		break;

	case EItemClass::Medicine:
		ApplyMedicineEffects(ItemRow);
		break;

	default:
		// На всякий случай: если ItemClass не выставлен, но StoreCategory намекает
		switch (ItemRow.StoreCategory)
		{
	case EStoreCategory::storecat_Food:
			ApplyFoodEffects(ItemRow);
			break;

	case EStoreCategory::storecat_Water:
			ApplyWaterEffects(ItemRow);
			break;

	case EStoreCategory::storecat_Medicine:
			ApplyMedicineEffects(ItemRow);
			break;

	default:
			// Просто применим базу без лишней логики
			ApplyConsumableBase(ItemRow.ConsumableEffects);
			break;
		}
		break;
	}
}

void UPlayerStatsComponent::ApplyFoodEffects(const FItemBaseRow& ItemRow)
{
	const FConsumableEffects& Eff = ItemRow.ConsumableEffects;

	// Базовые эффекты (хил, стамина, голод, жажда, и т.д.)
	ApplyConsumableBase(Eff);

	// Плохой эффект от еды: отравление и био-заражение
	if (Eff.BadEffectChance > 0.f && Eff.BadEffectPower > 0.f)
	{
		const float Roll = FMath::FRandRange(0.f, 100.f);
		if (Roll <= Eff.BadEffectChance)
		{
			// Просрочка: основной удар по Poisoning,
			// чуть добавляем Biohazard
			Secondary.Poisoning = FMath::Clamp(
				Secondary.Poisoning + Eff.BadEffectPower,
				0.f, 100.f);

			Secondary.Biohazard = FMath::Clamp(
				Secondary.Biohazard + Eff.BadEffectPower * 0.5f,
				0.f, 100.f);
		}
	}
}

void UPlayerStatsComponent::ApplyWaterEffects(const FItemBaseRow& ItemRow)
{
	const FConsumableEffects& Eff = ItemRow.ConsumableEffects;

	// Базовые эффекты: жажда, здоровье, радиация, Cold и т.д.
	ApplyConsumableBase(Eff);

	// Грязная вода: отравление + немного радиации
	if (Eff.BadEffectChance > 0.f && Eff.BadEffectPower > 0.f)
	{
		const float Roll = FMath::FRandRange(0.f, 100.f);
		if (Roll <= Eff.BadEffectChance)
		{
			Secondary.Poisoning = FMath::Clamp(
				Secondary.Poisoning + Eff.BadEffectPower,
				0.f, 100.f);

			Base.Radiation = FMath::Clamp(
				Base.Radiation + Eff.BadEffectPower * 0.3f,
				0.f, 100.f);
		}
	}
}

void UPlayerStatsComponent::ApplyMedicineEffects(const FItemBaseRow& ItemRow)
{
	const FConsumableEffects& Eff = ItemRow.ConsumableEffects;

	// Базовые эффекты:
	// - хил
	// - снижение Biohazard/Poisoning/PsyRad (задаёшь отрицательными дельтами)
	ApplyConsumableBase(Eff);

	// Передоз медикаментов:
	if (Eff.BadEffectChance > 0.f && Eff.BadEffectPower > 0.f)
	{
		const float Roll = FMath::FRandRange(0.f, 100.f);
		if (Roll <= Eff.BadEffectChance)
		{
			// Основной эффект — Poisoning (печень, почки),
			// можно добавить чуть PsyRad, если хочешь "психотропы"
			Secondary.Poisoning = FMath::Clamp(
				Secondary.Poisoning + Eff.BadEffectPower,
				0.f, 100.f);

			Secondary.PsyRad = FMath::Clamp(
				Secondary.PsyRad + Eff.BadEffectPower * 0.2f,
				0.f, 100.f);
		}
	}
}

void UPlayerStatsComponent::ApplyAmbientDamage(
	float Electro, float Fire, float Physical, float Cold, float Poison)
{
	Ambient.Electro   += FMath::Max(0.f, Electro);
	Ambient.Fire      += FMath::Max(0.f, Fire);
	Ambient.Physical  += FMath::Max(0.f, Physical);
	Ambient.Cold      += FMath::Max(0.f, Cold);
	Ambient.Poisoning += FMath::Max(0.f, Poison);

	// Суммарный урон
	const float TotalDamage = Electro + Fire + Physical + Cold + Poison;
	if (TotalDamage > 0.f)
	{
		ApplyHealthDamage(TotalDamage);
	}
}

void UPlayerStatsComponent::AddFood(float Value)
{
	Secondary.Food = FMath::Clamp(Secondary.Food + Value, 0.f, 100.f);
}

void UPlayerStatsComponent::AddWater(float Value)
{
	Secondary.Water = FMath::Clamp(Secondary.Water + Value, 0.f, 100.f);
}

void UPlayerStatsComponent::ConsumeStamina(float Amount)
{
	if (Amount <= 0.f)
		return;

	Base.Stamina = FMath::Max(0.f, Base.Stamina - Amount);
}

void UPlayerStatsComponent::RestoreStamina(float DeltaTime)
{
	// Если жажда >= 60% — стамина не восстанавливается
	if (Secondary.Water >= 60.f)
		return;

	// Если перегруз > 60% — медленный реген, иначе быстрый
	const float RegenRate = (Secondary.Weight > 60.f) ? 5.f : 10.f;

	Base.Stamina = FMath::Min(100.f, Base.Stamina + RegenRate * DeltaTime);
}

/* ====== Внутренняя логика ====== */
void UPlayerStatsComponent::TickGoodBadStats(float DeltaTime)
{
	// ====== Рост голода/жажды со временем ======
	if (bEnableNeeds)
	{
		float Mult = 1.0f;

		if (const ACharacter* Ch = Cast<ACharacter>(GetOwner()))
		{
			if (const UCharacterMovementComponent* Move = Ch->GetCharacterMovement())
			{
				const float Speed2D = Move->Velocity.Size2D();

				if (Speed2D > MoveSpeedThreshold)
				{
					Mult *= MoveNeedsMultiplier;
				}

				if (Move->MaxWalkSpeed > KINDA_SMALL_NUMBER &&
					Speed2D > Move->MaxWalkSpeed * SprintSpeedRatioThreshold)
				{
					Mult *= SprintNeedsMultiplier;
				}
			}
		}

		AddFood((FoodIncreasePerMinute / 60.0f) * DeltaTime * Mult);
		AddWater((WaterIncreasePerMinute / 60.0f) * DeltaTime * Mult);
	}

	// ====== Уурон при критических значениях ======
	// 80%+ → наносим урон
	// 60%+ → стамина не регенится
	if (Secondary.Water >= 80.f)
	{
		ApplyHealthDamage(3.f * DeltaTime);
	}

	// Food — 80%+ → наносим урон
	if (Secondary.Food >= 80.f)
	{
		ApplyHealthDamage(2.f * DeltaTime);
	}
}

// Ambient урон показываем только когда он есть, а потом быстро гасим
void UPlayerStatsComponent::TickAmbient(float DeltaTime)
{
	const float Decay = 40.f * DeltaTime;

	Ambient.Electro    = FMath::Max(0.f, Ambient.Electro    - Decay);
	Ambient.Fire       = FMath::Max(0.f, Ambient.Fire       - Decay);
	Ambient.Physical   = FMath::Max(0.f, Ambient.Physical   - Decay);
	Ambient.Cold       = FMath::Max(0.f, Ambient.Cold       - Decay);
	Ambient.Poisoning  = FMath::Max(0.f, Ambient.Poisoning  - Decay);
}

// Bleeding — если есть кровотечение, тикает урон
void UPlayerStatsComponent::TickBleeding(float DeltaTime)
{
	if (Secondary.Bleeding > 0.f)
	{
		ApplyHealthDamage(5.f * DeltaTime);
	}
}

// Weight — пока только влияет на реген стамины (см. RestoreStamina)
void UPlayerStatsComponent::TickWeight(float DeltaTime)
{
	// логика по скорости передвижения/анимкам — в Character (по желанию)
}

// Radiation — 25%+ → урон + повышает жажду
void UPlayerStatsComponent::TickRadiation(float DeltaTime)
{
	if (Base.Radiation >= 25.f)
	{
		ApplyHealthDamage(1.2f * DeltaTime);
		AddWater(4.f * DeltaTime);
	}
}

// PsyRad — 25%+ → урон по HP + жрёт стамину
void UPlayerStatsComponent::TickPsyHazards(float DeltaTime)
{
	if (Secondary.PsyRad >= 25.f)
	{
		ApplyHealthDamage(1.f * DeltaTime);
		Base.Stamina = FMath::Max(0.f, Base.Stamina - 8.f * DeltaTime);
	}
}

// Biohazard — 25%+ → урон + поднимает жажду/голод
void UPlayerStatsComponent::TickBioHazards(float DeltaTime)
{
	if (Secondary.Biohazard >= 25.f)
	{
		ApplyHealthDamage(1.5f * DeltaTime);
		AddFood(2.f * DeltaTime);
		AddWater(2.f * DeltaTime);
	}
}

void UPlayerStatsComponent::ClampAll()
{
	Base.Health    = FMath::Clamp(Base.Health,    0.f, 100.f);
	Base.Stamina   = FMath::Clamp(Base.Stamina,   0.f, 100.f);
	Base.Radiation = FMath::Clamp(Base.Radiation, 0.f, 100.f);

	Secondary.Biohazard = FMath::Clamp(Secondary.Biohazard, 0.f, 100.f);
	Secondary.PsyRad    = FMath::Clamp(Secondary.PsyRad,    0.f, 100.f);
	Secondary.Bleeding  = FMath::Clamp(Secondary.Bleeding,  0.f, 100.f);
	Secondary.Weight    = FMath::Clamp(Secondary.Weight,    0.f, 100.f);
	Secondary.Cold      = FMath::Clamp(Secondary.Cold,      0.f, 100.f);
	Secondary.Poisoning = FMath::Clamp(Secondary.Poisoning, 0.f, 100.f);
	Secondary.Food      = FMath::Clamp(Secondary.Food,      0.f, 100.f);
	Secondary.Water     = FMath::Clamp(Secondary.Water,     0.f, 100.f);
}

void UPlayerStatsComponent::ApplyConsumableBase(const FConsumableEffects& Eff)
{
	// Базовые хорошие/плохие эффекты, которые всегда применяются
	Base.Health    = FMath::Clamp(Base.Health    + Eff.HealthDelta,    0.f, 100.f);
	Base.Stamina   = FMath::Clamp(Base.Stamina   + Eff.StaminaDelta,   0.f, 100.f);
	Base.Radiation = FMath::Clamp(Base.Radiation + Eff.RadiationDelta, 0.f, 100.f);

	Secondary.Food      = FMath::Clamp(Secondary.Food      + Eff.FoodDelta,      0.f, 100.f);
	Secondary.Water     = FMath::Clamp(Secondary.Water     + Eff.WaterDelta,     0.f, 100.f);
	Secondary.Biohazard = FMath::Clamp(Secondary.Biohazard + Eff.BiohazardDelta, 0.f, 100.f);
	Secondary.PsyRad    = FMath::Clamp(Secondary.PsyRad    + Eff.PsyRadDelta,    0.f, 100.f);
	Secondary.Poisoning = FMath::Clamp(Secondary.Poisoning + Eff.PoisoningDelta, 0.f, 100.f);
	Secondary.Cold      = FMath::Clamp(Secondary.Cold      + Eff.ColdDelta,      0.f, 100.f);
}