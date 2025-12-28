#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "InventoryWidget.generated.h"

class UItemSlotWidget;
class UCanvasPanel;
class UUserWidget;
class UDragDropOperation;
class UItemTooltipWidget;
class UItemActionMenuWidget;
enum class EItemContextAction : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryItemEquipRequested, const FItemBaseRow&, ItemRow);

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

	/** Количество в стеке (если ItemRow.bUseStack) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;
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
	TSubclassOf<UItemSlotWidget> ItemWidgetClass;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ARES|Inventory")
	bool bPackFilteredTabs = false;

	// Сигнал наружу: "по этому предмету в инвентаре даблкликнули — хотим одеть"
	UPROPERTY(BlueprintAssignable, Category="ARES|Inventory")
	FOnInventoryItemEquipRequested OnItemEquipRequested;

	// Tooltip
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory|Tooltip")
	TSubclassOf<UItemTooltipWidget> TooltipWidgetClass;

	UPROPERTY(Transient)
	UItemTooltipWidget* ItemTooltipWidget = nullptr;

	// ===== Context Menu =====
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="ARES|Inventory|ContextMenu")
	TSubclassOf<UItemActionMenuWidget> ActionMenuWidgetClass;

	UPROPERTY(Transient)
	UItemActionMenuWidget* ItemActionMenuWidget = nullptr;

protected:
	virtual void NativeConstruct() override;

	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation) override;

	/** Построение пустых слотов, с учётом занятых ячеек предметами */
	void BuildEmptySlots(const TArray<FInventoryItemEntry>& SourceItems);

	/** Внутреннее построение по массиву предметов */
	void BuildFromItems(const TArray<FInventoryItemEntry>& SourceItems);

	/** Создать один виджет предмета и положить на Canvas */
	void CreateItemWidget(const FInventoryItemEntry& Entry);

	/** Fix отображения предметов в Фильтр-Категории */
	void PackItemsIntoLocalGrid(TArray<FInventoryItemEntry>& Items);

	UFUNCTION()
	void HandleItemSlotDoubleClicked(const FItemBaseRow& ItemRow);
	
public:
	// Tooltip
	void ShowItemTooltip(const FItemBaseRow& ItemRow, int32 Quantity, const FVector2D& ScreenPos);
	void ShowEquipmentTooltip(const FItemBaseRow& ItemRow, int32 Quantity, const FVector2D& ScreenPos);
	void HideItemTooltip();

	// Context Menu
	void ShowItemActionMenu(const FItemBaseRow& ItemRow, int32 CellX, int32 CellY, int32 Quantity, const FVector2D& ScreenPos);
	void ShowEquipmentActionMenu(const FItemBaseRow& ItemRow, EEquipmentSlotType SlotType, const FVector2D& ScreenPos);
	void HideItemActionMenu();

	// Context Menu Close
	void RequestCloseItemActionMenu();
	void CancelCloseItemActionMenu();

private:
	// Tooltip
	void EnsureTooltipCreated();
	void AttachTooltipToCanvas();
	void AttachTooltipToViewport();

	// Context Menu
	void EnsureActionMenuCreated();
	void AttachActionMenuToCanvas();
	void AttachActionMenuToViewport();

	UFUNCTION()
	void HandleContextAction(EItemContextAction Action);

	bool HasSubCategory(EStoreSubCategory SubCat) const;
	bool HasAnyAmmo() const;

	// текущий “выбранный” предмет для меню
	FName Menu_InternalName = NAME_None;
	int32 Menu_CellX = 0;
	int32 Menu_CellY = 0;
	int32 Menu_Quantity = 1;
	
	bool bMenuFromEquipment = false;
	EEquipmentSlotType Menu_EquipmentSlot = EEquipmentSlotType::None;

	// Context Menu Close
	FTimerHandle ActionMenuCloseTimer;
	float ActionMenuCloseDelay = 0.15f;
};