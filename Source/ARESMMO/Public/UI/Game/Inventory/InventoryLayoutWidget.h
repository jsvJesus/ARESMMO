#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "InventoryLayoutWidget.generated.h"

class AARESMMOCharacter;
class UWidgetSwitcher;
class UInventoryWidget;
struct FInventoryItemEntry;
class UImage;
class UTextureRenderTarget2D;

UCLASS()
class ARESMMO_API UInventoryLayoutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Здесь лежат 10 страниц (каждая — свой InventoryWidget) */
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* InventorySwitcher;

	// Каждая страница — инстанс того же класса UInventoryWidget
	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_All;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Weapon;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Armor;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Helmet;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Medic;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Food;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Items;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Devices;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Craft;

	UPROPERTY(meta=(BindWidgetOptional))
	UInventoryWidget* Inv_Attm;

	// Картинка персонажа в инвентаре
	UPROPERTY(meta=(BindWidget))
	UImage* PlayerRef;

	/** Вызывается из BP по клику на иконки вкладок (0..9) */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void ShowTab(int32 TabIndex);

	/** Раздать массив предметов всем вкладкам (каждый сам отфильтрует по FilterCategories) */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void DistributeItems(const TArray<FInventoryItemEntry>& AllItems);

protected:
	virtual void NativeConstruct() override;

	TWeakObjectPtr<AARESMMOCharacter> PreviewCharacter;
	bool bIsRotatingPreview = false;

	// Обработка мышки для вращения
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void InitPreview(class AARESMMOCharacter* Character);
	
	// Установить текстуру в PlayerRef
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void SetPlayerImage(UTextureRenderTarget2D* RenderTarget);
};