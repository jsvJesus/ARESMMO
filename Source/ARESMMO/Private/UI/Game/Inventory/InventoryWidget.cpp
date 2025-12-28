#include "UI/Game/Inventory/InventoryWidget.h"
#include "UI/Game/Inventory/Context/ItemDragDropOperation.h"
#include "UI/Game/Inventory/Context/ItemTooltipWidget.h"
#include "UI/Game/Inventory/ItemSlotWidget.h"
#include "UI/Game/Inventory/Context/ItemActionMenuWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Blueprint/UserWidget.h"
#include "ARESMMO/ARESMMOCharacter.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Weapons/WeaponBase.h"

void UInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RebuildInventory();
	EnsureTooltipCreated();
	EnsureActionMenuCreated();
}

bool UInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* Op = Cast<UItemDragDropOperation>(InOperation);
	if (!Op)
	{
		return false;
	}

	AARESMMOCharacter* Char = Cast<AARESMMOCharacter>(GetOwningPlayerPawn());
	if (!Char)
	{
		return false;
	}

	const FVector2D LocalPos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());

	int32 TargetCellX = FMath::FloorToInt(LocalPos.X / CellSize);
	int32 TargetCellY = FMath::FloorToInt(LocalPos.Y / CellSize);

	// если бросили мимо сетки — отбой
	if (TargetCellX < 0 || TargetCellX >= InventoryWidthCells || TargetCellY < 0 || TargetCellY >= InventoryHeightCells)
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("InventoryWidget Drop: Source=%d to (%d,%d)"),
		(int32)Op->SourceType, TargetCellX, TargetCellY);

	// Equipment -> Inventory
	if (Op->SourceType == EItemDragSource::Equipment)
	{
		return Char->UnequipSlotToInventoryAt(Op->FromSlot, TargetCellX, TargetCellY);
	}

	// Inventory -> Inventory
	if (Op->SourceType == EItemDragSource::Inventory)
	{
		return Char->MoveInventoryItem(Op->ItemRow.InternalName, Op->FromCellX, Op->FromCellY, TargetCellX, TargetCellY);
	}

	return false;
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

	// Tooltip
	EnsureTooltipCreated();

	// Context menu
	EnsureActionMenuCreated();
	HideItemActionMenu();

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
	if (bUseFilter && bPackFilteredTabs)
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

	ItemWidget->InitItem(Entry.ItemRow, Entry.SizeInCells, Entry.CellX, Entry.CellY, Entry.Quantity);
	ItemWidget->SetOwnerInventory(this);
	ItemWidget->OnItemDoubleClicked.AddDynamic(this, &UInventoryWidget::HandleItemSlotDoubleClicked);

	const float PosX = Entry.CellX * CellSize;
	const float PosY = Entry.CellY * CellSize;
	const float SizeX = Entry.SizeInCells.Width  * CellSize;
	const float SizeY = Entry.SizeInCells.Height * CellSize;

	InvSlot->SetAutoSize(false);
	InvSlot->SetPosition(FVector2D(PosX, PosY));
	InvSlot->SetSize(FVector2D(SizeX, SizeY));
	InvSlot->SetZOrder(10);             // поверх пустых

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

void UInventoryWidget::ShowItemTooltip(const FItemBaseRow& ItemRow, int32 Quantity, const FVector2D& ScreenPos)
{
	EnsureTooltipCreated();
	if (!ItemTooltipWidget || !InventoryCanvas)
	{
		return;
	}

	AttachTooltipToCanvas();
	ItemTooltipWidget->SetTooltipData(ItemRow, Quantity);
	ItemTooltipWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

	ItemTooltipWidget->ForceLayoutPrepass();

	UCanvasPanelSlot* TootipSlot = Cast<UCanvasPanelSlot>(ItemTooltipWidget->Slot);
	if (!TootipSlot)
	{
		return;
	}

	const FGeometry& CanvasGeo = InventoryCanvas->GetCachedGeometry();
	FVector2D Local = CanvasGeo.AbsoluteToLocal(ScreenPos) + FVector2D(16.f, 16.f);

	const FVector2D CanvasSize = CanvasGeo.GetLocalSize();
	FVector2D Desired = ItemTooltipWidget->GetDesiredSize();
	if (Desired.IsNearlyZero())
	{
		Desired = FVector2D(320.f, 200.f);
	}

	Local.X = FMath::Clamp(Local.X, 0.f, FMath::Max(0.f, CanvasSize.X - Desired.X));
	Local.Y = FMath::Clamp(Local.Y, 0.f, FMath::Max(0.f, CanvasSize.Y - Desired.Y));

	TootipSlot->SetPosition(Local);
}

void UInventoryWidget::ShowEquipmentTooltip(const FItemBaseRow& ItemRow, int32 Quantity, const FVector2D& ScreenPos)
{
	EnsureTooltipCreated();
	if (!ItemTooltipWidget)
	{
		return;
	}

	AttachTooltipToViewport();
	ItemTooltipWidget->SetTooltipData(ItemRow, Quantity);
	ItemTooltipWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	ItemTooltipWidget->ForceLayoutPrepass();

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
	FVector2D Desired = ItemTooltipWidget->GetDesiredSize();
	if (Desired.IsNearlyZero())
	{
		Desired = FVector2D(320.f, 200.f);
	}

	FVector2D Position = ScreenPos + FVector2D(16.f, 16.f);
	Position.X = FMath::Clamp(Position.X, 0.f, FMath::Max(0.f, ViewportSize.X - Desired.X));
	Position.Y = FMath::Clamp(Position.Y, 0.f, FMath::Max(0.f, ViewportSize.Y - Desired.Y));

	ItemTooltipWidget->SetPositionInViewport(Position, false);
}

void UInventoryWidget::HideItemTooltip()
{
	if (ItemTooltipWidget)
	{
		ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventoryWidget::ShowItemActionMenu(const FItemBaseRow& ItemRow, int32 CellX, int32 CellY, int32 Quantity,
	const FVector2D& ScreenPos)
{
	CancelCloseItemActionMenu();
	EnsureActionMenuCreated();
	
	if (!ItemActionMenuWidget || !InventoryCanvas)
	{
		return;
	}

	// запомнили контекст
	Menu_InternalName = ItemRow.InternalName;
	Menu_CellX = CellX;
	Menu_CellY = CellY;
	Menu_Quantity = Quantity;

	bMenuFromEquipment = false;
	Menu_EquipmentSlot = EEquipmentSlotType::None;

	// ресурсы в инвентаре
	const bool bHasBattery   = HasSubCategory(EStoreSubCategory::Item_Battery);
	const bool bHasRepairKit = HasSubCategory(EStoreSubCategory::Item_RapairKit);
	const bool bHasAmmo      = HasAnyAmmo();

	ItemActionMenuWidget->SetupForItem(ItemRow, bHasBattery, bHasAmmo, bHasRepairKit);
	if (AARESMMOCharacter* Char = Cast<AARESMMOCharacter>(GetOwningPlayerPawn()))
	{
		TSet<EStoreSubCategory> DetachOptions;
		AWeaponBase* Weapon = Char->GetSelectedWeapon();
		if (!Weapon)
		{
			Weapon = Char->GetBestWeaponForAttachment();
		}

		if (Weapon)
		{
			for (const auto& Pair : Weapon->AttachedATTM)
			{
				DetachOptions.Add(Pair.Key);
			}
		}

		ItemActionMenuWidget->SetDetachOptions(DetachOptions);
	}
	ItemActionMenuWidget->SetVisibility(ESlateVisibility::Visible);
	ItemActionMenuWidget->ForceLayoutPrepass();

	UCanvasPanelSlot* MenuSlot = Cast<UCanvasPanelSlot>(ItemActionMenuWidget->Slot);
	if (!MenuSlot)
	{
		return;
	}

	const FGeometry& CanvasGeo = InventoryCanvas->GetCachedGeometry();
	FVector2D Local = CanvasGeo.AbsoluteToLocal(ScreenPos) + FVector2D(8.f, 8.f);

	const FVector2D CanvasSize = CanvasGeo.GetLocalSize();
	FVector2D Desired = ItemActionMenuWidget->GetDesiredSize();
	if (Desired.IsNearlyZero())
	{
		Desired = FVector2D(220.f, 260.f);
	}

	Local.X = FMath::Clamp(Local.X, 0.f, FMath::Max(0.f, CanvasSize.X - Desired.X));
	Local.Y = FMath::Clamp(Local.Y, 0.f, FMath::Max(0.f, CanvasSize.Y - Desired.Y));

	MenuSlot->SetPosition(Local);
}

void UInventoryWidget::ShowEquipmentActionMenu(
	const FItemBaseRow& ItemRow,
	EEquipmentSlotType SlotType,
	const FVector2D& ScreenPos)
{
	CancelCloseItemActionMenu();
	EnsureActionMenuCreated();

	if (!ItemActionMenuWidget)
	{
		return;
	}

	AttachActionMenuToViewport();
	Menu_InternalName = ItemRow.InternalName;
	Menu_CellX = 0;
	Menu_CellY = 0;
	Menu_Quantity = 1;
	bMenuFromEquipment = true;
	Menu_EquipmentSlot = SlotType;

	ItemActionMenuWidget->SetupForEquipmentItem(ItemRow);
	ItemActionMenuWidget->SetVisibility(ESlateVisibility::Visible);
	ItemActionMenuWidget->ForceLayoutPrepass();

	const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
	FVector2D Desired = ItemActionMenuWidget->GetDesiredSize();
	if (Desired.IsNearlyZero())
	{
		Desired = FVector2D(220.f, 260.f);
	}

	FVector2D Position = ScreenPos + FVector2D(8.f, 8.f);
	Position.X = FMath::Clamp(Position.X, 0.f, FMath::Max(0.f, ViewportSize.X - Desired.X));
	Position.Y = FMath::Clamp(Position.Y, 0.f, FMath::Max(0.f, ViewportSize.Y - Desired.Y));

	ItemActionMenuWidget->SetPositionInViewport(Position, false);
}

void UInventoryWidget::HideItemActionMenu()
{
	CancelCloseItemActionMenu();
	
	if (ItemActionMenuWidget)
	{
		ItemActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	bMenuFromEquipment = false;
	Menu_EquipmentSlot = EEquipmentSlotType::None;
}

void UInventoryWidget::RequestCloseItemActionMenu()
{
	if (!ItemActionMenuWidget) return;

	// если уже закрыто — не трогаем
	if (ItemActionMenuWidget->GetVisibility() != ESlateVisibility::Visible)
		return;

	// запланировать закрытие через небольшую задержку
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(ActionMenuCloseTimer);
		W->GetTimerManager().SetTimer(ActionMenuCloseTimer, this, &UInventoryWidget::HideItemActionMenu, ActionMenuCloseDelay, false);
	}
}

void UInventoryWidget::CancelCloseItemActionMenu()
{
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(ActionMenuCloseTimer);
	}
}

void UInventoryWidget::EnsureTooltipCreated()
{
	if (!InventoryCanvas)
	{
		return;
	}

	if (!ItemTooltipWidget && TooltipWidgetClass)
	{
		ItemTooltipWidget = CreateWidget<UItemTooltipWidget>(GetWorld(), TooltipWidgetClass);
	}

	if (ItemTooltipWidget && !ItemTooltipWidget->GetParent())
	{
		UCanvasPanelSlot* TootipSlot = InventoryCanvas->AddChildToCanvas(ItemTooltipWidget);
		if (TootipSlot)
		{
			TootipSlot->SetAutoSize(true);
			TootipSlot->SetZOrder(999); // всегда поверх всего
		}

		ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
		ItemTooltipWidget->SetIsEnabled(false); // чтобы не перехватывал ввод
	}
}

void UInventoryWidget::AttachTooltipToCanvas()
{
	if (!ItemTooltipWidget || !InventoryCanvas)
	{
		return;
	}

	if (ItemTooltipWidget->GetParent() != InventoryCanvas)
	{
		ItemTooltipWidget->RemoveFromParent();
		UCanvasPanelSlot* TootipSlot = InventoryCanvas->AddChildToCanvas(ItemTooltipWidget);
		if (TootipSlot)
		{
			TootipSlot->SetAutoSize(true);
			TootipSlot->SetZOrder(1000);
		}
		ItemTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
		ItemTooltipWidget->SetIsEnabled(false);
	}
}

void UInventoryWidget::AttachTooltipToViewport()
{
	if (!ItemTooltipWidget)
	{
		return;
	}

	if (!ItemTooltipWidget->IsInViewport())
	{
		ItemTooltipWidget->RemoveFromParent();
		ItemTooltipWidget->AddToViewport(1000);
	}
}

void UInventoryWidget::EnsureActionMenuCreated()
{
	if (!InventoryCanvas)
	{
		UE_LOG(LogTemp, Error, TEXT("EnsureActionMenuCreated: InventoryCanvas = NULL"));
		return;
	}

	if (!ActionMenuWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("EnsureActionMenuCreated: ActionMenuWidgetClass = NULL (set it in BP!)"));
		return;
	}

	if (!ItemActionMenuWidget)
	{
		ItemActionMenuWidget = CreateWidget<UItemActionMenuWidget>(GetWorld(), ActionMenuWidgetClass);

		if (ItemActionMenuWidget)
		{
			ItemActionMenuWidget->SetOwnerInventory(this);
		}
		
		UE_LOG(LogTemp, Warning, TEXT("EnsureActionMenuCreated: created menu %d"), ItemActionMenuWidget ? 1 : 0);
	}

	if (ItemActionMenuWidget && !ItemActionMenuWidget->GetParent())
	{
		UCanvasPanelSlot* ActionSlot = InventoryCanvas->AddChildToCanvas(ItemActionMenuWidget);
		if (ActionSlot)
		{
			ActionSlot->SetAutoSize(true);
			ActionSlot->SetZOrder(1000);
			UE_LOG(LogTemp, Warning, TEXT("EnsureActionMenuCreated: added to canvas"));
		}

		ItemActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
		ItemActionMenuWidget->OnActionSelected.AddDynamic(this, &UInventoryWidget::HandleContextAction);
	}
}

void UInventoryWidget::AttachActionMenuToCanvas()
{
	if (!ItemActionMenuWidget || !InventoryCanvas)
	{
		return;
	}

	if (ItemActionMenuWidget->GetParent() != InventoryCanvas)
	{
		ItemActionMenuWidget->RemoveFromParent();
		UCanvasPanelSlot* ActionSlot = InventoryCanvas->AddChildToCanvas(ItemActionMenuWidget);
		if (ActionSlot)
		{
			ActionSlot->SetAutoSize(true);
			ActionSlot->SetZOrder(1000);
		}
		ItemActionMenuWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInventoryWidget::AttachActionMenuToViewport()
{
	if (!ItemActionMenuWidget)
	{
		return;
	}

	if (!ItemActionMenuWidget->IsInViewport())
	{
		ItemActionMenuWidget->RemoveFromParent();
		ItemActionMenuWidget->AddToViewport(1000);
	}
}

void UInventoryWidget::HandleContextAction(EItemContextAction Action)
{
	AARESMMOCharacter* Char = Cast<AARESMMOCharacter>(GetOwningPlayerPawn());
	if (!Char)
	{
		HideItemActionMenu();
		return;
	}

	if (bMenuFromEquipment)
	{
		if (Action == EItemContextAction::Equip && Menu_EquipmentSlot != EEquipmentSlotType::None)
		{
			Char->UnequipSlot(Menu_EquipmentSlot);
		}

		HideItemActionMenu();
		return;
	}

	switch (Action)
	{
	case EItemContextAction::Equip:
		Char->ContextMenu_Equip(Menu_InternalName, Menu_CellX, Menu_CellY);
		break;

	case EItemContextAction::Attach:
		Char->ContextMenu_Attach(Menu_InternalName, Menu_CellX, Menu_CellY);
		break;

	case EItemContextAction::Detach:
		if (ItemActionMenuWidget)
		{
			const EStoreSubCategory SubCategory = ItemActionMenuWidget->ConsumePendingDetachSubCategory();
			if (SubCategory != EStoreSubCategory::None)
			{
				Char->DetachWeaponATTMToInventory(SubCategory);
			}
		}
		break;

	case EItemContextAction::Use:
		Char->ContextMenu_Use(Menu_InternalName, Menu_CellX, Menu_CellY);
		break;

	case EItemContextAction::Study:
		Char->ContextMenu_Study(Menu_InternalName, Menu_CellX, Menu_CellY);
		break;

	case EItemContextAction::Drop:
		Char->ContextMenu_Drop(Menu_InternalName, Menu_CellX, Menu_CellY);
		break;

	case EItemContextAction::ChargeItem:
		Char->ContextMenu_ChargeItem(Menu_InternalName, Menu_CellX, Menu_CellY);
		break;

	case EItemContextAction::ChargeMagazine:
		Char->ContextMenu_ChargeMagazine(Menu_InternalName, Menu_CellX, Menu_CellY);
		break;

	case EItemContextAction::Repair:
		Char->ContextMenu_Repair(Menu_InternalName, Menu_CellX, Menu_CellY);
		break;

	default:
		break;
	}

	HideItemActionMenu();
}

bool UInventoryWidget::HasSubCategory(EStoreSubCategory SubCat) const
{
	for (const FInventoryItemEntry& Entry : AllItems)
	{
		if (Entry.ItemRow.StoreSubCategory == SubCat)
		{
			return true;
		}
	}
	return false;
}

bool UInventoryWidget::HasAnyAmmo() const
{
	for (const FInventoryItemEntry& Entry : AllItems)
	{
		if (Entry.ItemRow.StoreCategory == EStoreCategory::storecat_Ammo)
		{
			return true;
		}
	}
	return false;
}
