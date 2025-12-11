#include "UI/Game/Inventory/InventoryWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/UserWidget.h"
#include "UI/Game/Inventory/ItemSlotWidget.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

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
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget: InventoryCanvas is null"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("InventoryWidget::RebuildInventory: AllItems=%d, FilterCategories=%d"),
		AllItems.Num(), FilterCategories.Num());

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

	//  - вкладка "All" (без фильтра) показывает реальные координаты из инвентаря
	//  - остальные вкладки уплотняем с (0,0)
	if (bUseFilter)
	{
		PackItemsIntoLocalGrid(Filtered);
	}

	UE_LOG(LogTemp, Log, TEXT("InventoryWidget::RebuildInventory: Filtered=%d"), Filtered.Num());

	BuildEmptySlots(Filtered);
	BuildFromItems(Filtered);
}

void UInventoryWidget::BuildEmptySlots(const TArray<FInventoryItemEntry>& SourceItems)
{
	if (!InventoryCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget::BuildEmptySlots: InventoryCanvas is null"));
		return;
	}

	if (!EmptySlotWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryWidget::BuildEmptySlots: EmptySlotWidgetClass is NONE"));
		return;
	}

	const int32 WidthCells  = InventoryWidthCells;
	const int32 HeightCells = InventoryHeightCells;

	UE_LOG(LogTemp, Log, TEXT("BuildEmptySlots: Width=%d Height=%d SourceItems=%d"),
		WidthCells, HeightCells, SourceItems.Num());

	if (WidthCells <= 0 || HeightCells <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BuildEmptySlots: invalid grid size"));
		return;
	}

	TArray<bool> Occupied;
	Occupied.Init(false, WidthCells * HeightCells);

	auto MarkCellOccupied = [&](int32 X, int32 Y)
	{
		if (X < 0 || X >= WidthCells || Y < 0 || Y >= HeightCells)
		{
			return;
		}
		const int32 Index = Y * WidthCells + X;
		Occupied[Index] = true;
	};

	for (const FInventoryItemEntry& Entry : SourceItems)
	{
		const int32 StartX = Entry.CellX;
		const int32 StartY = Entry.CellY;
		const int32 SizeX  = Entry.SizeInCells.Width;
		const int32 SizeY  = Entry.SizeInCells.Height;

		for (int32 LocalX = 0; LocalX < SizeX; ++LocalX)
		{
			for (int32 LocalY = 0; LocalY < SizeY; ++LocalY)
			{
				MarkCellOccupied(StartX + LocalX, StartY + LocalY);
			}
		}
	}

	int32 CreatedSlots = 0;

	for (int32 Y = 0; Y < HeightCells; ++Y)
	{
		for (int32 X = 0; X < WidthCells; ++X)
		{
			const int32 Index = Y * WidthCells + X;
			if (Occupied[Index])
			{
				continue;
			}

			UUserWidget* SlotWidget = CreateWidget<UUserWidget>(GetWorld(), EmptySlotWidgetClass);
			if (!SlotWidget)
			{
				continue;
			}

			UCanvasPanelSlot* CanvasSlot = InventoryCanvas->AddChildToCanvas(SlotWidget);
			if (!CanvasSlot)
			{
				continue;
			}

			const float PosX = static_cast<float>(X) * CellSize;
			const float PosY = static_cast<float>(Y) * CellSize;

			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetPosition(FVector2D(PosX, PosY));
			CanvasSlot->SetSize(FVector2D(CellSize, CellSize));
			CanvasSlot->SetZOrder(-1);

			++CreatedSlots;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("BuildEmptySlots: CreatedSlots=%d"), CreatedSlots);
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
	if (!InventoryCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateItemWidget: InventoryCanvas is null"));
		return;
	}

	if (!ItemWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateItemWidget: ItemWidgetClass is not set"));
		return;
	}

	// создаём КОНКРЕТНО UItemSlotWidget
	UItemSlotWidget* ItemWidget = CreateWidget<UItemSlotWidget>(GetWorld(), ItemWidgetClass);
	if (!ItemWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateItemWidget: failed to create ItemSlotWidget"));
		return;
	}

	UCanvasPanelSlot* InvSlot = InventoryCanvas->AddChildToCanvas(ItemWidget);
	if (!InvSlot)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateItemWidget: AddChildToCanvas failed"));
		return;
	}

	UItemSlotWidget* ItemWidget = CreateWidget<UItemSlotWidget>(GetWorld(), ItemWidgetClass);
	ItemWidget->InitItem(Entry.ItemRow, Entry.SizeInCells);
	ItemWidget->OnItemDoubleClicked.AddDynamic(this, &UInventoryWidget::HandleItemSlotDoubleClicked);

	const float PosX = Entry.CellX * CellSize;
	const float PosY = Entry.CellY * CellSize;
	const float SizeX = Entry.SizeInCells.Width  * CellSize;
	const float SizeY = Entry.SizeInCells.Height * CellSize;

	InvSlot->SetAutoSize(false);
	InvSlot->SetPosition(FVector2D(PosX, PosY));
	InvSlot->SetSize(FVector2D(SizeX, SizeY));
	InvSlot->SetZOrder(10);             // поверх пустых

	// ВАЖНО: говорим слоту какой это предмет и какой размер в клетках
	ItemWidget->InitItem(Entry.ItemRow, Entry.SizeInCells);

	UE_LOG(LogTemp, Log, TEXT("CreateItemWidget: %s at (%d,%d) size %dx%d"),
		*Entry.ItemRow.InternalName.ToString(),
		Entry.CellX, Entry.CellY,
		Entry.SizeInCells.Width, Entry.SizeInCells.Height);
}

void UInventoryWidget::PackItemsIntoLocalGrid(TArray<FInventoryItemEntry>& Items)
{
	const int32 WidthCells  = InventoryWidthCells;
	const int32 HeightCells = InventoryHeightCells;

	if (WidthCells <= 0 || HeightCells <= 0)
	{
		return;
	}

	TArray<bool> Occupied;
	Occupied.Init(false, WidthCells * HeightCells);

	auto CanPlace = [&](int32 StartX, int32 StartY, const FInventoryItemEntry& Entry) -> bool
	{
		const int32 SizeX = Entry.SizeInCells.Width;
		const int32 SizeY = Entry.SizeInCells.Height;

		for (int32 LocalX = 0; LocalX < SizeX; ++LocalX)
		{
			for (int32 LocalY = 0; LocalY < SizeY; ++LocalY)
			{
				const int32 X = StartX + LocalX;
				const int32 Y = StartY + LocalY;

				if (X < 0 || X >= WidthCells || Y < 0 || Y >= HeightCells)
				{
					return false;
				}

				const int32 Index = Y * WidthCells + X;
				if (Occupied[Index])
				{
					return false;
				}
			}
		}
		return true;
	};

	auto MarkPlaced = [&](int32 StartX, int32 StartY, const FInventoryItemEntry& Entry)
	{
		const int32 SizeX = Entry.SizeInCells.Width;
		const int32 SizeY = Entry.SizeInCells.Height;

		for (int32 LocalX = 0; LocalX < SizeX; ++LocalX)
		{
			for (int32 LocalY = 0; LocalY < SizeY; ++LocalY)
			{
				const int32 X = StartX + LocalX;
				const int32 Y = StartY + LocalY;
				const int32 Index = Y * WidthCells + X;
				Occupied[Index] = true;
			}
		}
	};

	for (FInventoryItemEntry& Entry : Items)
	{
		bool bPlaced = false;

		for (int32 Y = 0; Y < HeightCells && !bPlaced; ++Y)
		{
			for (int32 X = 0; X < WidthCells && !bPlaced; ++X)
			{
				if (CanPlace(X, Y, Entry))
				{
					Entry.CellX = X;
					Entry.CellY = Y;

					MarkPlaced(X, Y, Entry);
					bPlaced = true;
				}
			}
		}

		if (!bPlaced)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("PackItemsIntoLocalGrid: cannot place item %s"),
				*Entry.ItemRow.InternalName.ToString());
		}
	}
}

void UInventoryWidget::HandleItemSlotDoubleClicked(const FItemBaseRow& ItemRow)
{
	OnItemEquipRequested.Broadcast(ItemRow);
}
