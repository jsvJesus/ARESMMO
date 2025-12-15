#pragma once

#include "CoreMinimal.h"
#include "UI/Game/Inventory/ItemSlotWidget.h"
#include "ItemDragWidget.generated.h"

class UItemDragDropWidget;

UCLASS()
class ARESMMO_API UItemDragWidget : public UItemSlotWidget
{
	GENERATED_BODY()

public:
	// Визуал, который летит за курсором
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory|DragDrop")
	TSubclassOf<UItemDragDropWidget> DragVisualClass;

	// px размер клетки (по умолчанию 64)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory|DragDrop")
	float CellSizePx = 64.f;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeConstruct() override;

	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation) override;
};