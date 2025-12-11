#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "Styling/SlateTypes.h"
#include "InventoryLayoutWidget.generated.h"

class AARESMMOCharacter;
class UWidgetSwitcher;
class UInventoryWidget;
struct FInventoryItemEntry;
class UImage;
class UTextureRenderTarget2D;
class UButton;
class UEquipmentWidget;

UCLASS()
class ARESMMO_API UInventoryLayoutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Здесь лежат 10 страниц (каждая — свой InventoryWidget) */
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* InventorySwitcher;

	// Панель экипировки
	UPROPERTY(meta=(BindWidgetOptional))
	UEquipmentWidget* EquipmentPanel;

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

	/** --------- КНОПКИ КАТЕГОРИЙ ---------
	 *  Имена ДОЛЖНЫ совпадать с именами Button в UMG
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_All;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Weapon;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Armor;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Helmet;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Medic;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Food;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Items;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Devices;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Craft;

	UPROPERTY(meta = (BindWidgetOptional))
	UButton* Btn_Attm;

	/** Текущая активная вкладка (0..9) */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Inventory|Tabs")
	int32 CurrentTabIndex = 0;

	// БАЗОВЫЕ стили (общие для всех вкладок)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	FButtonStyle TabNormalBaseStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	FButtonStyle TabActiveBaseStyle;

	// ИКОНКИ ДЛЯ КАЖДОЙ ВКЛАДКИ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_All = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Weapon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Armor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Helmet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Medic = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Food = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Items = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Devices = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Craft = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Inventory|Tabs")
	UTexture2D* Icon_Attm = nullptr;

	// Картинка персонажа в инвентаре
	UPROPERTY(meta=(BindWidget))
	UImage* PlayerRef;

	/** Вызывается из BP по клику на иконки вкладок (0..9) */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void ShowTab(int32 TabIndex);

	/** Обновить визуал кнопок вкладок */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void UpdateTabButtons();

	/** Раздать массив предметов всем вкладкам (каждый сам отфильтрует по FilterCategories) */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void DistributeItems(const TArray<FInventoryItemEntry>& AllItems);

	/** Функция для передачи мапы экипировки */
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void SetEquipment(const TMap<EEquipmentSlotType, FItemBaseRow>& Equipment);

	UFUNCTION()
	void HandleUnequipRequested(EEquipmentSlotType SlotType);

	UFUNCTION()
	void HandleInventoryItemEquipRequested(const FItemBaseRow& ItemRow);

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