#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Items/ItemData.h"
#include "Items/ItemTypes.h"
#include "ItemDragDropOperation.generated.h"

class UImage;
class UTextBlock;
/** Данные для Drag&Drop предметов в инвентаре/экипировке */
UCLASS()
class ARESMMO_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** Предмет, который тащим */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Inventory")
	FItemBaseRow ItemRow;

	/** Размер предмета в клетках */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Inventory")
	FItemSize ItemSize;

	/** Стартовая позиция (для перемещения внутри инвентаря) */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Inventory")
	int32 SourceCellX = 0;

	UPROPERTY(BlueprintReadOnly, Category="ARES|Inventory")
	int32 SourceCellY = 0;

	/** Флаг: предмет тащат из экипировки */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Inventory")
	bool bFromEquipment = false;

	/** Если предмет из экипировки — из какого слота */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Inventory")
	EEquipmentSlotType SourceEquipmentSlot = EEquipmentSlotType::None;
};