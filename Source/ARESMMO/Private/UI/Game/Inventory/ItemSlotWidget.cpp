#include "UI/Game/Inventory/ItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/ItemConditionLibrary.h"
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

// одна клетка = 64 px
static constexpr float CELL_PX = 64.f;

void UItemSlotWidget::InitItem(const FItemBaseRow& ItemRow, const FItemSize& GridSize)
{
	CurrentItemRow = ItemRow;
	
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

	// ---------- СТЕК ----------
	if (StackText)
	{
		if (ItemRow.bUseStackSize && ItemRow.MaxStackSize > 1)
		{
			const int32 Count = FMath::Clamp(ItemRow.StackSize, 1, ItemRow.MaxStackSize);
			StackText->SetText(FText::FromString(FString::Printf(TEXT("x%d"), Count)));
			StackText->SetVisibility(Count > 1 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		}
		else
		{
			StackText->SetText(FText::GetEmpty());
			StackText->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	// ---------- ВЕС ПРЕДМЕТА (с учётом ТЕКУЩЕГО StackSize) ----------
	if (WeightText)
	{
		const int32 Count = (ItemRow.bUseStackSize ? FMath::Max(1, ItemRow.StackSize) : 1);
		const float EffectiveWeight = ItemRow.Weight * static_cast<float>(Count);

		WeightText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), EffectiveWeight)));
	}

	// ---------- ПРОЧНОСТЬ ПРЕДМЕТА (Durability %) ----------
	/*const bool bUsesDurability =
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
	*/

	if (ConditionText)
	{
		const bool bUsesDurability = (ItemRow.bUseRepair && ItemRow.MaxDurability > 0);

		if (bUsesDurability)
		{
			const int32 Max = ItemRow.MaxDurability;
			int32 Current = ItemRow.CurrDurability;

			// Если ты где-то ещё не инициализируешь CurrDurability — можно фолбекнуть на Default
			if (Current <= 0 && ItemRow.DefaultDurability > 0)
			{
				Current = ItemRow.DefaultDurability;
			}

			Current = FMath::Clamp(Current, 0, Max);

			const int32 PctInt = FMath::RoundToInt((static_cast<float>(Current) / static_cast<float>(Max)) * 100.f);
			ConditionText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PctInt)));

			const EItemConditionState State = UItemConditionLibrary::GetConditionStateFromValues(Current, Max);
			ConditionText->SetColorAndOpacity(UItemConditionLibrary::GetConditionColor(State));
		}
		else
		{
			ConditionText->SetText(FText::GetEmpty());
		}
	}

	// ---------- ЗАРЯД ПРЕДМЕТА (Charge %) по подкатегориям ----------
	/*const bool bUsesCharge =
		(ItemRow.StoreSubCategory == EStoreSubCategory::Usable_PDA          ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::Usable_Detector    ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Laser   ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Flashlight ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::GearATTM_NVG       ||
		 ItemRow.StoreSubCategory == EStoreSubCategory::GearATTM_Headlamp);
	*/

	if (ChargeText)
	{
		const bool bUsesCharge = (ItemRow.bUseCharge && ItemRow.MaxCharge > 0);

		if (bUsesCharge)
		{
			const int32 Max = ItemRow.MaxCharge;
			int32 Current = ItemRow.CurrCharge;

			if (Current <= 0 && ItemRow.DefaultCharge > 0)
			{
				Current = ItemRow.DefaultCharge;
			}

			Current = FMath::Clamp(Current, 0, Max);

			const int32 PctInt = FMath::RoundToInt((static_cast<float>(Current) / static_cast<float>(Max)) * 100.f);
			ChargeText->SetText(FText::FromString(FString::Printf(TEXT("%d%%"), PctInt)));

			const EItemConditionState State = UItemConditionLibrary::GetConditionStateFromValues(Current, Max);
			ChargeText->SetColorAndOpacity(UItemConditionLibrary::GetConditionColor(State));
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