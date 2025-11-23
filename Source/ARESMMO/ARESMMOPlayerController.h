#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ARESMMOPlayerController.generated.h"

class UInventoryWidget;
class UGameHUDWidget;

UCLASS()
class ARESMMO_API AARESMMOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AARESMMOPlayerController();

protected:
	/** Класс виджета HUD (Blueprint на основе UGameHUDWidget). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UGameHUDWidget> GameHUDClass;

	/** Инстанс HUD, добавленный на экран. */
	UPROPERTY(Transient)
	TObjectPtr<UGameHUDWidget> GameHUDInstance = nullptr;

	/** Класс виджета инвентаря (Blueprint на основе UInventoryWidget). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"))
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

	/** Инстанс инвентаря. */
	UPROPERTY(Transient)
	TObjectPtr<UInventoryWidget> InventoryWidgetInstance = nullptr;

	/** Флаг, открыт ли сейчас инвентарь. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI", meta=(AllowPrivateAccess="true"))
	bool bInventoryVisible = false;

protected:
	virtual void BeginPlay() override;

	/** Спаун HUD один раз при старте. */
	void CreateGameHUD();

	/** Спаун (или только показать) инвентаря. */
	void ShowInventory();

	/** Скрыть инвентарь. */
	void HideInventory();

public:
	/** Переключить состояние инвентаря (открыть/закрыть). Вызывается по кнопке "I". */
	UFUNCTION(BlueprintCallable, Category="UI")
	void ToggleInventory();

	/** Доступ к HUD (если вдруг пригодится). */
	UFUNCTION(BlueprintPure, Category="UI")
	UGameHUDWidget* GetGameHUD() const { return GameHUDInstance; }

	/** Доступ к инвентарю (если нужно что-то из BP). */
	UFUNCTION(BlueprintPure, Category="UI")
	UInventoryWidget* GetInventoryWidget() const { return InventoryWidgetInstance; }

	/** Узнать, открыт ли инвентарь. */
	UFUNCTION(BlueprintPure, Category="UI")
	bool IsInventoryVisible() const { return bInventoryVisible; }
};