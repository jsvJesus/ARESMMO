#include "UI/Game/Inventory/InventoryWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/UserWidget.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// При создании можем сразу перерисовать (если AllItems уже заполнен)
	RebuildInventory();
}

void UInventoryWidget::SetAllItems(const TArray<FInventoryItemEntry>& NewItems)
{
	AllItems = NewItems;
	RebuildInventory();
}

void UInventoryWidget::RebuildInventory()
{
	if (!InventoryCanvas)
	{
		return;
	}

	InventoryCanvas->ClearChildren();

	TArray<FInventoryItemEntry> Filtered;

	const bool bUseFilter = FilterCategories.Num() > 0;

	if (bUseFilter)
	{
		for (const FInventoryItemEntry& Entry : AllItems)
		{
			if (FilterCategories.Contains(Entry.ItemRow.StoreCategory))
			{
				Filtered.Add(Entry);
			}
		}
	}
	else
	{
		Filtered = AllItems;
	}

	BuildFromItems(Filtered);
}

void UInventoryWidget::BuildFromItems(const TArray<FInventoryItemEntry>& SourceItems)
{
	for (const FInventoryItemEntry& Entry : SourceItems)
	{
		CreateItemWidget(Entry);
	}
}

void UInventoryWidget::CreateItemWidget(const FInventoryItemEntry& Entry)
{
	if (!InventoryCanvas || !ItemWidgetClass)
	{
		return;
	}

	UUserWidget* ItemWidget = CreateWidget<UUserWidget>(GetWorld(), ItemWidgetClass);
	if (!ItemWidget)
	{
		return;
	}

	UCanvasPanelSlot* InvSlot = InventoryCanvas->AddChildToCanvas(ItemWidget);
	if (!InvSlot)
	{
		return;
	}

	// Позиция в пикселях
	const float PosX = Entry.CellX * CellSize;
	const float PosY = Entry.CellY * CellSize;

	// Размер в пикселях
	const float SizeX = Entry.SizeInCells.Width  * CellSize;
	const float SizeY = Entry.SizeInCells.Height * CellSize;

	InvSlot->SetAutoSize(false);
	InvSlot->SetPosition(FVector2D(PosX, PosY));
	InvSlot->SetSize(FVector2D(SizeX, SizeY));
}