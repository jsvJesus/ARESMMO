#include "UI/Game/Inventory/ItemSlotWidget.h"
#include "UI/Game/Inventory/Context/ItemDragDropOperation.h"
#include "UI/Game/Inventory/InventoryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Items/ItemConditionLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"
#include "Engine/Texture2D.h"

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UItemSlotWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnItemDoubleClicked.Broadcast(CurrentItemRow);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

FReply UItemSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// ЛКМ: Drag&Drop
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	// ПКМ: контекстное меню
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("RMB ItemSlotWidget: %s  OwnerInv=%d"),
			*CurrentItemRow.InternalName.ToString(),
			OwnerInventory.IsValid() ? 1 : 0);

		if (OwnerInventory.IsValid())
		{
			OwnerInventory->CancelCloseItemActionMenu();
			OwnerInventory->HideItemTooltip();
			OwnerInventory->ShowItemActionMenu(CurrentItemRow, CachedCellX, CachedCellY, CachedQuantity, InMouseEvent.GetScreenSpacePosition());
		}
		return FReply::Handled();
	}
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UItemSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	UItemDragDropOperation* Op = NewObject<UItemDragDropOperation>();
	Op->SourceType = EItemDragSource::Inventory;
	Op->ItemRow    = CurrentItemRow;
	Op->SizeInCells = CachedSize;
	Op->Quantity    = CachedQuantity;
	Op->FromCellX   = CachedCellX;
	Op->FromCellY   = CachedCellY;

	// Drag visual (чтобы тащить красивую иконку + фон)
	UItemSlotWidget* DragVisual = CreateWidget<UItemSlotWidget>(GetOwningPlayer(), GetClass());
	if (DragVisual)
	{
		DragVisual->InitItem(CurrentItemRow, CachedSize, CachedCellX, CachedCellY, CachedQuantity);
		DragVisual->SetRenderOpacity(0.85f);

		// ВАЖНО: DragVisual вне Canvas, поэтому задаём ему размер вручную,
		// иначе виджет схлопнется под текст, а картинки (Background/Icon) не будут видны.
		const float CellPx = 64.f;
		const float Wpx = static_cast<float>(CachedSize.Width) * CellPx;
		const float Hpx = static_cast<float>(CachedSize.Height) * CellPx;

		USizeBox* DragBox = NewObject<USizeBox>(Op);
		if (DragBox)
		{
			DragBox->SetWidthOverride(Wpx);
			DragBox->SetHeightOverride(Hpx);
			DragBox->SetContent(DragVisual);

			Op->DefaultDragVisual = DragBox;
		}
		else
		{
			// fallback
			Op->DefaultDragVisual = DragVisual;
		}

		Op->Pivot = EDragPivot::MouseDown;
	}

	OutOperation = Op;

	UE_LOG(LogTemp, Log, TEXT("ItemSlotWidget DragStart: %s from (%d,%d)"),
		*CurrentItemRow.InternalName.ToString(), CachedCellX, CachedCellY);

	// Скрываем Tooltip при Drag&Drop
	if (OwnerInventory.IsValid())
	{
		OwnerInventory->HideItemTooltip();
	}
}

void UItemSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	if (OwnerInventory.IsValid())
	{
		OwnerInventory->CancelCloseItemActionMenu();
		OwnerInventory->ShowItemTooltip(CurrentItemRow, CachedQuantity, InMouseEvent.GetScreenSpacePosition());
	}

	UE_LOG(LogTemp, Warning, TEXT("HOVER ENTER: %s"), *CurrentItemRow.InternalName.ToString());
}

void UItemSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	if (OwnerInventory.IsValid())
	{
		OwnerInventory->HideItemTooltip();
		OwnerInventory->RequestCloseItemActionMenu();
	}
}

// одна клетка = 64 px
static constexpr float CELL_PX = 64.f;

void UItemSlotWidget::InitItem(const FItemBaseRow& ItemRow, const FItemSize& GridSize, int32 InCellX, int32 InCellY, int32 InQuantity)
{
	CurrentItemRow   = ItemRow;
	CachedItemRow    = ItemRow;
	CachedSize       = GridSize;
	CachedCellX      = InCellX;
	CachedCellY      = InCellY;
	CachedQuantity   = InQuantity;
	
	const int32 W = GridSize.Width;
	const int32 H = GridSize.Height;

	// ---------- ФОН ----------
	if (BackgroundImage)
	{
		if (UTexture2D* BG = GetBackgroundTexture(GridSize))
		{
			UE_LOG(LogTemp, Log, TEXT("BG loaded: %s"), *BG->GetName());
			BackgroundImage->SetBrushFromTexture(BG, true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BG tex not found for %dx%d"), W, H);
		}

		FSlateBrush Brush = BackgroundImage->GetBrush();
		Brush.ImageSize = FVector2D(W * CELL_PX, H * CELL_PX);
		BackgroundImage->SetBrush(Brush);

		// делаем фон полностью непрозрачным, чтобы убить сетку сзади
		BackgroundImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f));
	}

	// ---------- ИКОНКА ПРЕДМЕТА ----------
	if (IconImage)
	{
		if (!ItemRow.Icon.IsNull())
		{
			if (UTexture2D* IconTex = ItemRow.Icon.LoadSynchronous())
			{
				IconImage->SetBrushFromTexture(IconTex, true);

				FSlateBrush IconBrush = IconImage->GetBrush();
				IconBrush.ImageSize = FVector2D(W * CELL_PX, H * CELL_PX);
				IconImage->SetBrush(IconBrush);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ItemSlotWidget: Icon load failed for %s"),
					*ItemRow.InternalName.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ItemSlotWidget: Icon is NULL for %s"),
				*ItemRow.InternalName.ToString());
		}
	}

	// ---------- ИМЯ ПРЕДМЕТА ----------
	if (NameText)
	{
		NameText->SetText(ItemRow.DisplayName);
	}

	// ---------- ВЕС ПРЕДМЕТА (с учётом MaxStackSize там, где надо) ----------
	if (WeightText)
	{
		float EffectiveWeight = ItemRow.Weight;

		// Категории, где используем MaxStackSize
		switch (ItemRow.StoreCategory)
		{
		case EStoreCategory::storecat_Backpack:
		case EStoreCategory::storecat_Grenade:
		case EStoreCategory::storecat_Medicine:
		case EStoreCategory::storecat_Food:
		case EStoreCategory::storecat_Water:
		case EStoreCategory::storecat_Components:
		case EStoreCategory::storecat_CraftRecipes:
		case EStoreCategory::storecat_CraftItems:
		case EStoreCategory::storecat_Ammo:
		case EStoreCategory::storecat_WeaponATTM:
		case EStoreCategory::storecat_GearATTM:
			EffectiveWeight = ItemRow.Weight * FMath::Max(1, ItemRow.MaxStackSize);
			break;

		default:
			// для всего остального – вес одного предмета
			break;
		}

		const FString WeightStr = FString::Printf(TEXT("%.1f"), EffectiveWeight);
		WeightText->SetText(FText::FromString(WeightStr));
	}

	// ---------- ПРОЧНОСТЬ ПРЕДМЕТА (Durability %) ----------
	const bool bUsesDurability =
		(ItemRow.StoreCategory == EStoreCategory::storecat_Armor  ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_Helmet ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_Mask   ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_ASR    ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_SNP    ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_SHTG   ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_HG     ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_MG     ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_SMG    ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_MELEE  ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_CraftItems ||
		 ItemRow.StoreCategory == EStoreCategory::storecat_PlaceItem);

	if (ConditionText)
	{
		if (bUsesDurability && ItemRow.MaxDurability > 0)
		{
			const int32 Current = ItemRow.DefaultDurability;
			const int32 Max     = ItemRow.MaxDurability;

			const float Pct    = static_cast<float>(Current) / static_cast<float>(Max);
			const int32 PctInt = FMath::RoundToInt(Pct * 100.f);

			ConditionText->SetText(FText::FromString(
				FString::Printf(TEXT("%d%%"), PctInt)));

			// Цвет через библиотеку (порогa из ItemConditionLibrary)
			const EItemConditionState State =
				UItemConditionLibrary::GetConditionStateFromValues(Current, Max);

			const FLinearColor Color =
				UItemConditionLibrary::GetConditionColor(State);

			ConditionText->SetColorAndOpacity(Color);
		}
		else
		{
			ConditionText->SetText(FText::GetEmpty());
		}
	}

	// ---------- ЗАРЯД ПРЕДМЕТА (Charge %) по подкатегориям ----------
	const bool bUsesCharge =
		(ItemRow.StoreSubCategory == EStoreSubCategory::Usable_PDA          ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::Usable_Detector    ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Laser   ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Flashlight ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::GearATTM_NVG       ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::GearATTM_Headlamp);

	if (ChargeText)
	{
		if (bUsesCharge && ItemRow.MaxCharge > 0)
		{
			const int32 Current = ItemRow.DefaultCharge;
			const int32 Max     = ItemRow.MaxCharge;

			const float Pct    = static_cast<float>(Current) / static_cast<float>(Max);
			const int32 PctInt = FMath::RoundToInt(Pct * 100.f);

			ChargeText->SetText(FText::FromString(
				FString::Printf(TEXT("%d%%"), PctInt)));

			const EItemConditionState State =
				UItemConditionLibrary::GetConditionStateFromValues(Current, Max);

			const FLinearColor Color =
				UItemConditionLibrary::GetConditionColor(State);

			ChargeText->SetColorAndOpacity(Color);
		}
		else
		{
			ChargeText->SetText(FText::GetEmpty());
		}
	}
}

UTexture2D* LoadTex(const TCHAR* Path)
{
	return LoadObject<UTexture2D>(nullptr, Path);
}

UTexture2D* UItemSlotWidget::GetBackgroundTexture(const FItemSize& GridSize) const
{
	const int32 W = GridSize.Width;
	const int32 H = GridSize.Height;

	// Полная таблица размеров → текcтур
	if (W == 1 && H == 1)
		return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/64x64.64x64"));

	if (W == 1 && H == 2)
		return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/64x128.64x128"));

	if (W == 2 && H == 1)
		return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/128x64.128x64"));

	if (W == 2 && H == 2)
		return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/128x128.128x128"));

	//if (W == 2 && H == 3)
		//return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/128x192.128x192"));

	if (W == 2 && H == 4)
		return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/128x256.128x256"));

	//if (W == 3 && H == 2)
		//return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/192x128.192x128"));

	if (W == 3 && H == 3)
		return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/192x192.192x192"));

	if (W == 4 && H == 2)
		return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/256x128.256x128"));

	// Фолбек
	return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/64x64.64x64"));
}

void UItemSlotWidget::SetOwnerInventory(UInventoryWidget* InOwner)
{
	OwnerInventory = InOwner;
}
