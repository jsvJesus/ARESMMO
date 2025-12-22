#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "ItemSlotWidget.generated.h"

class UTextBlock;
class UImage;
class UInventoryWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemDoubleClicked, const FItemBaseRow&, ItemRow);

UCLASS()
class ARESMMO_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Сигнал наружу: даблклик по предмету
	UPROPERTY(BlueprintAssignable, Category="ARES|Inventory")
	FOnInventoryItemDoubleClicked OnItemDoubleClicked;

	/** Фон предмета (64x64 / 128x128 / 128x256 / 192x192 / 256x128 и т.д.) */
	UPROPERTY(meta=(BindWidget))
	UImage* BackgroundImage;

	/** Иконка предмета */
	UPROPERTY(meta=(BindWidget))
	UImage* IconImage;

	/** Устанавливается из InventoryWidget::CreateItemWidget */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void InitItem(const FItemBaseRow& ItemRow, const FItemSize& GridSize, int32 InCellX, int32 InCellY, int32 InQuantity);

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* WeightText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ConditionText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ChargeText;

protected:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDoubleClick(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// Храним текущий предмет
	UPROPERTY()
	FItemBaseRow CurrentItemRow;

private:
	// Выбор правильного фона по размеру предмета
	UTexture2D* GetBackgroundTexture(const FItemSize& GridSize) const;

	FItemBaseRow CachedItemRow;
	FItemSize    CachedSize;
	
	int32 CachedCellX = 0;
	int32 CachedCellY = 0;
	int32 CachedQuantity = 1;

	TWeakObjectPtr<UInventoryWidget> OwnerInventory;

public:
	void SetOwnerInventory(UInventoryWidget* InOwner);
};