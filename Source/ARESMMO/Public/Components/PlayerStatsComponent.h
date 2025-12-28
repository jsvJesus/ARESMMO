#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemData.h"
#include "PlayerStatsComponent.generated.h"

/* ===== Основные статы ===== */
USTRUCT(BlueprintType)
struct FBaseStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Base")
	float Health = 100.0f;   // 0..100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Base")
	float Stamina = 100.0f;  // 0..100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Base")
	float Radiation = 0.0f;  // 0..100
};

/* ===== Второстепенные статы ===== */
USTRUCT(BlueprintType)
struct FSecondaryStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Secondary")
	float Biohazard = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Secondary")
	float PsyRad = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Secondary")
	float Bleeding = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Secondary")
	float Weight = 0.0f; // % от перегруза

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Secondary")
	float Cold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Secondary")
	float Poisoning = 0.0f;

	// 0% — сыт, 100% — голодный до смерти
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Secondary")
	float Food = 0.0f;

	// 0% — напился, 100% — умирает от жажды
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Secondary")
	float Water = 0.0f;
};

/* ===== Ambient статы (урон от аномалий) ===== */
USTRUCT(BlueprintType)
struct FAmbientStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ambient")
	float Electro = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ambient")
	float Fire = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ambient")
	float Physical = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ambient")
	float Cold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Ambient")
	float Poisoning = 0.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ARESMMO_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStatsComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	/* Пакеты статов */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	FBaseStats Base;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	FSecondaryStats Secondary;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	FAmbientStats Ambient;

	/* ===== Публичное API ===== */

	// Прямой урон по здоровью
	UFUNCTION(BlueprintCallable, Category="Stats")
	void ApplyHealthDamage(float Damage);

	/* Эффект от употребления медикаментов/еды/воды */
	UFUNCTION(BlueprintCallable, Category="ARES|Stats")
	void ApplyConsumable(const FItemBaseRow& ItemRow);

	/** Точечные функции, если нужно вызывать напрямую */
	UFUNCTION(BlueprintCallable, Category="ARES|Stats")
	void ApplyFoodEffects(const FItemBaseRow& ItemRow);

	UFUNCTION(BlueprintCallable, Category="ARES|Stats")
	void ApplyWaterEffects(const FItemBaseRow& ItemRow);

	UFUNCTION(BlueprintCallable, Category="ARES|Stats")
	void ApplyMedicineEffects(const FItemBaseRow& ItemRow);

	// Урон от аномалий
	UFUNCTION(BlueprintCallable, Category="Stats")
	void ApplyAmbientDamage(float Electro, float Fire, float Physical, float Cold, float Poison);

	// Изменение голода / жажды (Value может быть <0 или >0)
	UFUNCTION(BlueprintCallable, Category="Stats")
	void AddFood(float Value);

	UFUNCTION(BlueprintCallable, Category="Stats")
	void AddWater(float Value);

	// Стамина
	UFUNCTION(BlueprintCallable, Category="Stats")
	void ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category="Stats")
	void RestoreStamina(float DeltaTime);

	// ===== Needs (Food/Water grow over time) =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Needs")
	bool bEnableNeeds = true;

	// Сколько % в минуту набегает (0..100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Needs", meta=(ClampMin="0.0"))
	float FoodIncreasePerMinute = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Needs", meta=(ClampMin="0.0"))
	float WaterIncreasePerMinute = 1.5f;

	// Множители, если персонаж двигается/бежит
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Needs", meta=(ClampMin="1.0"))
	float MoveNeedsMultiplier = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Needs", meta=(ClampMin="1.0"))
	float SprintNeedsMultiplier = 1.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Needs", meta=(ClampMin="0.0"))
	float MoveSpeedThreshold = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats|Needs", meta=(ClampMin="0.0", ClampMax="1.0"))
	float SprintSpeedRatioThreshold = 0.75f;

private:
	void TickGoodBadStats(float DeltaTime);  // Food / Water
	void TickAmbient(float DeltaTime);       // Electro, Fire, Cold, Poisoning
	void TickBleeding(float DeltaTime);      // Bleeding
	void TickWeight(float DeltaTime);        // Weight (сейчас пустышка, логика в регене стамины)
	void TickRadiation(float DeltaTime);     // Radiation
	void TickPsyHazards(float DeltaTime);    // PsyRad
	void TickBioHazards(float DeltaTime);    // Biohazard

	void ClampAll();                         // clamp 0..100

	/** Общая часть: применяет Health/Stamina/Food/Water/Poisoning и т.д. по структуре эффектов */
	void ApplyConsumableBase(const FConsumableEffects& Eff);
};