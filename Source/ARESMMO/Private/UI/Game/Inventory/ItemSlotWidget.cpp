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
	CurrentItemRow = ItemRow;
	CachedItemRow  = ItemRow;
	CachedSize     = GridSize;
	CachedCellX    = InCellX;
	CachedCellY    = InCellY;
	CachedQuantity = InQuantity;

	const int32 W = GridSize.Width;
	const int32 H = GridSize.Height;

	// ---------- ФОН ----------
	if (BackgroundImage)
	{
		if (UTexture2D* BG = GetBackgroundTexture(GridSize))
		{
			BackgroundImage->SetBrushFromTexture(BG, true);
		}

		FSlateBrush Brush = BackgroundImage->GetBrush();
		Brush.ImageSize = FVector2D(W * CELL_PX, H * CELL_PX);
		BackgroundImage->SetBrush(Brush);

		// фон непрозрачный, чтобы не просвечивала сетка
		BackgroundImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f));
	}

	// ---------- ИКОНКА ----------
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
		}
	}

	// ---------- ТЕКСТЫ ----------
	auto SetTextOrCollapse = [](UTextBlock* TB, const FText& Text)
	{
		if (!TB) return;
		if (Text.IsEmpty())
		{
			TB->SetText(FText::GetEmpty());
			TB->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			TB->SetText(Text);
			TB->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	};

	const int32 Qty = FMath::Max(1, InQuantity);
	const EStoreCategory    Cat = ItemRow.StoreCategory;
	const EStoreSubCategory Sub = ItemRow.StoreSubCategory;

	// --- Типы ---
	const bool bIsMagazine  = (Sub == EStoreSubCategory::WeaponATTM_Magazine);
	const bool bIsRepairKit = (Sub == EStoreSubCategory::Item_RapairKit);
	const bool bIsBattery   = (Sub == EStoreSubCategory::Item_Battery);

	// Батарейные (проценты)
	const bool bBatteryLike =
		(Cat == EStoreCategory::storecat_UsableItem) ||
		(Sub == EStoreSubCategory::Usable_PDA) ||
		(Sub == EStoreSubCategory::Usable_Detector) ||
		(Sub == EStoreSubCategory::WeaponATTM_Laser) ||
		(Sub == EStoreSubCategory::WeaponATTM_Flashlight) ||
		(Sub == EStoreSubCategory::GearATTM_NVG) ||
		(Sub == EStoreSubCategory::GearATTM_Headlamp) ||
		bIsBattery;

	// --- Категории “прочность + вес” ---
	auto IsDurabilityCategory = [](EStoreCategory InCat) -> bool
	{
		switch (InCat)
		{
		case EStoreCategory::storecat_ASR:
		case EStoreCategory::storecat_SNP:
		case EStoreCategory::storecat_SHTG:
		case EStoreCategory::storecat_HG:
		case EStoreCategory::storecat_MG:
		case EStoreCategory::storecat_SMG:
		case EStoreCategory::storecat_MELEE:
		case EStoreCategory::storecat_Armor:
		case EStoreCategory::storecat_Helmet:
		case EStoreCategory::storecat_PlaceItem:
		case EStoreCategory::storecat_CraftItems:
			return true;
		default:
			return false;
		}
	};

	const bool bShowDurability =
		IsDurabilityCategory(Cat) &&
		ItemRow.bUseDurability &&
		ItemRow.MaxDurability > 0;

	// ---------- QUANTITY (в слоте используем NameText как "xN") ----------
	bool bShowQuantity = false;

	// Backpack: только стек, вес НЕ показываем
	if (Cat == EStoreCategory::storecat_Backpack)
	{
		bShowQuantity = true;
	}
	// Кол-во + вес
	else if (Cat == EStoreCategory::storecat_HeroParts ||
			 Cat == EStoreCategory::storecat_Mask ||
			 Cat == EStoreCategory::storecat_Grenade ||
			 Cat == EStoreCategory::storecat_Medicine ||
			 Cat == EStoreCategory::storecat_Food ||
			 Cat == EStoreCategory::storecat_Water ||
			 Cat == EStoreCategory::storecat_Components ||
			 Cat == EStoreCategory::storecat_CraftRecipes ||
			 Cat == EStoreCategory::storecat_Ammo)
	{
		bShowQuantity = true;
	}
	// Attachments: кол-во только если НЕ батарейные и НЕ магазин/не ремкит
	else if ((Cat == EStoreCategory::storecat_WeaponATTM || Cat == EStoreCategory::storecat_GearATTM) &&
		     !bBatteryLike && !bIsMagazine && !bIsRepairKit)
	{
		bShowQuantity = true;
	}

	// Магазин: если пустой — показываем стек (xN), если есть патроны — стек скрываем
	if (bIsMagazine && ItemRow.bUseAmmo && ItemRow.MaxAmmo > 0)
	{
		const bool bHasAmmo = (ItemRow.CurrAmmo > 0);
		bShowQuantity = !bHasAmmo;
	}

	// “прочность+вес” — количество не показываем
	if (bShowDurability)
	{
		bShowQuantity = false;
	}

	// Пустой магазин и Qty=1 — нет смысла показывать "x1"
	if (bShowQuantity && Qty <= 1)
	{
		// оставим так: x1 можно скрыть
		// bShowQuantity = false;
	}

	// ---------- WEIGHT ----------
	bool bShowWeight = ItemRow.bUseWeight && (Cat != EStoreCategory::storecat_Backpack);
	const float UnitWeight  = ItemRow.GetUnitWeightKg();
	const float TotalWeight = UnitWeight * static_cast<float>(Qty);

	// ---------- APPLY: NameText (xN) ----------
	if (NameText)
	{
		SetTextOrCollapse(NameText, bShowQuantity ? FText::FromString(FString::Printf(TEXT("x%d"), Qty)) : FText::GetEmpty());
	}

	// ---------- APPLY: WeightText ----------
	if (WeightText)
	{
		if (bShowWeight && TotalWeight > 0.f)
		{
			SetTextOrCollapse(WeightText, FText::FromString(FString::Printf(TEXT("%.2f kg"), TotalWeight)));
		}
		else
		{
			SetTextOrCollapse(WeightText, FText::GetEmpty());
		}
	}

	// ---------- APPLY: Durability % ----------
	if (ConditionText)
	{
		if (bShowDurability)
		{
			const int32 Current = ItemRow.DefaultDurability;
			const int32 Max     = ItemRow.MaxDurability;

			const float Pct    = (Max > 0) ? (static_cast<float>(Current) / static_cast<float>(Max)) : 0.f;
			const int32 PctInt = FMath::Clamp(FMath::RoundToInt(Pct * 100.f), 0, 100);

			SetTextOrCollapse(ConditionText, FText::FromString(FString::Printf(TEXT("%d%%"), PctInt)));

			const EItemConditionState State = UItemConditionLibrary::GetConditionStateFromValues(Current, Max);
			ConditionText->SetColorAndOpacity(UItemConditionLibrary::GetConditionColor(State));
		}
		else
		{
			SetTextOrCollapse(ConditionText, FText::GetEmpty());
		}
	}

	// ---------- APPLY: Ammo / Charge ----------
	if (ChargeText)
	{
		// Magazine: Ammo X/Y (только если есть патроны > 0)
		if (bIsMagazine && ItemRow.bUseAmmo && ItemRow.MaxAmmo > 0)
		{
			if (ItemRow.CurrAmmo > 0)
			{
				const int32 Current = ItemRow.CurrAmmo;
				const int32 Max     = ItemRow.MaxAmmo;

				SetTextOrCollapse(ChargeText, FText::FromString(FString::Printf(TEXT("%d/%d"), Current, Max)));

				const EItemConditionState State = UItemConditionLibrary::GetConditionStateFromValues(Current, Max);
				ChargeText->SetColorAndOpacity(UItemConditionLibrary::GetConditionColor(State));
			}
			else
			{
				SetTextOrCollapse(ChargeText, FText::GetEmpty());
			}
		}
		// RepairKit: uses (число)
		else if (bIsRepairKit && ItemRow.bUseCharge && ItemRow.MaxCharge > 0)
		{
			SetTextOrCollapse(ChargeText, FText::AsNumber(ItemRow.DefaultCharge));
		}
		// Battery-like: percent 0..100
		else if (bBatteryLike && ItemRow.bUseCharge && ItemRow.MaxCharge > 0)
		{
			const int32 Current = ItemRow.DefaultCharge;
			const int32 Max     = ItemRow.MaxCharge;

			const float Pct    = static_cast<float>(Current) / static_cast<float>(Max);
			const int32 PctInt = FMath::Clamp(FMath::RoundToInt(Pct * 100.f), 0, 100);

			SetTextOrCollapse(ChargeText, FText::FromString(FString::Printf(TEXT("%d%%"), PctInt)));

			const EItemConditionState State = UItemConditionLibrary::GetConditionStateFromValues(Current, Max);
			ChargeText->SetColorAndOpacity(UItemConditionLibrary::GetConditionColor(State));
		}
		else
		{
			SetTextOrCollapse(ChargeText, FText::GetEmpty());
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
