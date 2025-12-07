#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "InventoryWidget.generated.h"

class UCanvasPanel;
class UUserWidget;

/** Одна запись предмета в инвентаре (простая версия) */
USTRUCT(BlueprintType)
struct FInventoryItemEntry
{
	GENERATED_BODY()

	/** Строка из DataTable с данными предмета */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemBaseRow ItemRow;

	/** Размер предмета в клетках (Width x Height) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FItemSize SizeInCells;

	/** Позиция в клетках */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellX = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellY = 0;
};

UCLASS()
class ARESMMO_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Canvas, куда кладём спавнящиеся виджеты предметов */
	UPROPERTY(meta=(BindWidget))
	UCanvasPanel* InventoryCanvas;

	/** Виджет одного предмета (UMG — только картинка) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory")
	TSubclassOf<UUserWidget> ItemWidgetClass;

	/** Виджет пустого слота 64x64 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory")
	TSubclassOf<UUserWidget> EmptySlotWidgetClass;

	/** Категории, которые этот виджет показывает.
	 *  Пусто = показывать все предметы.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ARES|Inventory")
	TArray<EStoreCategory> FilterCategories;

	/** Все предметы, которые пришли в этот инвентарь (до фильтрации) */
	UPROPERTY(BlueprintReadOnly, Category="ARES|Inventory")
	TArray<FInventoryItemEntry> AllItems;

	/** Размер одной клетки */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory")
	float CellSize = 64.0f;

	/** Размер инвентаря в клетках */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory")
	int32 InventoryWidthCells = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory")
	int32 InventoryHeightCells = 11;

	/** Устанавливаем полный список предметов (из компоненты инвентаря) */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void SetAllItems(const TArray<FInventoryItemEntry>& NewItems);

	/** Перерисовать виджет (с учётом FilterCategories) */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void RebuildInventory();

protected:
	virtual void NativeConstruct() override;

	/** Построение пустых слотов, с учётом занятых ячеек предметами */
	void BuildEmptySlots(const TArray<FInventoryItemEntry>& SourceItems);

	/** Внутреннее построение по массиву предметов */
	void BuildFromItems(const TArray<FInventoryItemEntry>& SourceItems);

	/** Создать один виджет предмета и положить на Canvas */
	void CreateItemWidget(const FInventoryItemEntry& Entry);
};