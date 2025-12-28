#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"      // FItemBaseRow, GridSize
#include "Items/ItemTypes.h"     // EEquipmentSlotType
#include "EquipmentSlotWidget.generated.h"

class UTextBlock;
class UImage;
class USizeBox;
class UDragDropOperation;
class UInventoryWidget;
class UInventoryLayoutWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentSlotDoubleClicked, UEquipmentSlotWidget*, SlotWidget);

UCLASS()
class ARESMMO_API UEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Какой именно это слот (Armor, Helmet, Weapon1 и т.п.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ARES|Equipment")
	EEquipmentSlotType SlotType = EEquipmentSlotType::None;

	// Коробка, у которой будем менять размеры под размер предмета (в клетках * 64)
	UPROPERTY(meta=(BindWidgetOptional))
	USizeBox* SlotSizeBox;

	// Фон слота (рамка)
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* SlotBackground;

	// Иконка надетого предмета
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* ItemIcon;

	// Текущий предмет в этом слоте (копия строки из DataTable)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Equipment")
	FItemBaseRow CurrentItemRow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="ARES|Equipment")
	bool bHasItem = false;

	// Размер одной клетки в пикселях (как в обычном инвентаре)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ARES|Equipment")
	float CellSizePx = 64.0f;

	// Сигнал "по этому слоту даблкликнули"
	UPROPERTY(BlueprintAssignable, Category="ARES|Equipment")
	FOnEquipmentSlotDoubleClicked OnSlotDoubleClicked;

protected:
	// Реакция на двойной клик мышью
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Drag start
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// Create drag op
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

	// Drop handler
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
public:
	virtual void NativeConstruct() override;

	// Установить предмет (по ссылке)
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void SetItem(const FItemBaseRow& ItemRow);

	// Очистить слот
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void ClearItem();

	UFUNCTION(BlueprintPure, Category="ARES|Equipment")
	bool IsEmpty() const { return !bHasItem; }

private:
	UInventoryWidget* ResolveInventoryWidget() const;
};