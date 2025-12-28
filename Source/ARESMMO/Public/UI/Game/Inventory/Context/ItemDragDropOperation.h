#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Items/ItemData.h"
#include "ItemDragDropOperation.generated.h"

UENUM(BlueprintType)
enum class EItemDragSource : uint8
{
	Inventory UMETA(DisplayName="Inventory"),
	Equipment UMETA(DisplayName="Equipment"),
};

UCLASS()
class ARESMMO_API UItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category="ARES|DragDrop")
	EItemDragSource SourceType = EItemDragSource::Inventory;

	UPROPERTY(BlueprintReadWrite, Category="ARES|DragDrop")
	FItemBaseRow ItemRow;

	UPROPERTY(BlueprintReadWrite, Category="ARES|DragDrop")
	FItemSize SizeInCells;

	UPROPERTY(BlueprintReadWrite, Category="ARES|DragDrop")
	int32 Quantity = 1;

	// Inventory source
	UPROPERTY(BlueprintReadWrite, Category="ARES|DragDrop|Inventory")
	int32 FromCellX = 0;

	UPROPERTY(BlueprintReadWrite, Category="ARES|DragDrop|Inventory")
	int32 FromCellY = 0;

	// Equipment source
	UPROPERTY(BlueprintReadWrite, Category="ARES|DragDrop|Equipment")
	EEquipmentSlotType FromSlot = EEquipmentSlotType::None;
};