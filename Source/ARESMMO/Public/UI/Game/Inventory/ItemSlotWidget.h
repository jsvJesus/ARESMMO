#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "ItemSlotWidget.generated.h"

class UTextBlock;
class UImage;
class UItemDragDropOperation;
struct FInventoryItemEntry;

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
	void InitItem(const FInventoryItemEntry& Entry);

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* WeightText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ConditionText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ChargeText;

	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* StackText;

	void SetFromEquipmentSlot(EEquipmentSlotType InSlotType) { SourceEquipmentSlot = InSlotType; bFromEquipment = true; }

protected:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDoubleClick(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDown(
				const FGeometry& InGeometry,
				const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(
			const FGeometry& InGeometry,
			const FPointerEvent& InMouseEvent,
			UDragDropOperation*& OutOperation) override;

	// Храним текущий предмет
	UPROPERTY()
	FItemBaseRow CurrentItemRow;

	UPROPERTY()
	FItemSize CurrentItemSize;

	int32 CurrentCellX = 0;
	int32 CurrentCellY = 0;

	bool bFromEquipment = false;
	EEquipmentSlotType SourceEquipmentSlot = EEquipmentSlotType::None;

private:
	// Выбор правильного фона по размеру предмета
	UTexture2D* GetBackgroundTexture(const FItemSize& GridSize) const;

	FItemBaseRow CachedItemRow;
	FItemSize    CachedSize;
};