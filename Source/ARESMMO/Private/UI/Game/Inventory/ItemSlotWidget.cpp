#include "UI/Game/Inventory/ItemSlotWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UItemSlotWidget::InitItem(const FItemBaseRow& ItemRow, const FItemSize& GridSize)
{
	if (BackgroundImage)
	{
		if (UTexture2D* BG = GetBackgroundTexture(GridSize))
		{
			BackgroundImage->SetBrushFromTexture(BG, true);
		}
	}

	if (IconImage && ItemRow.Icon.IsValid())
	{
		IconImage->SetBrushFromTexture(ItemRow.Icon.LoadSynchronous(), true);
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