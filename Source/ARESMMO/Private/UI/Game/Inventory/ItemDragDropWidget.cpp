#include "UI/Game/Inventory/ItemDragDropWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

static UTexture2D* LoadTex(const TCHAR* Path)
{
	return LoadObject<UTexture2D>(nullptr, Path);
}

void UItemDragDropWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// DragVisual не должен ловить клики
	SetIsEnabled(false);

	if (BackgroundImage) BackgroundImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (IconImage)       IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}

UTexture2D* UItemDragDropWidget::GetBackgroundTexture(const FItemSize& GridSize) const
{
	const int32 W = GridSize.Width;
	const int32 H = GridSize.Height;

	if (W == 1 && H == 1) return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/64x64.64x64"));
	if (W == 1 && H == 2) return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/64x128.64x128"));
	if (W == 2 && H == 1) return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/128x64.128x64"));
	if (W == 2 && H == 2) return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/128x128.128x128"));
	if (W == 2 && H == 4) return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/128x256.128x256"));
	if (W == 3 && H == 3) return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/192x192.192x192"));
	if (W == 4 && H == 2) return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/256x128.256x128"));

	return LoadTex(TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/64x64.64x64"));
}

void UItemDragDropWidget::InitDragVisual(const FItemBaseRow& ItemRow, const FItemSize& SizeInCells, float CellPx)
{
	const int32 W = SizeInCells.Width;
	const int32 H = SizeInCells.Height;

	if (BackgroundImage)
	{
		if (UTexture2D* BG = GetBackgroundTexture(SizeInCells))
		{
			BackgroundImage->SetBrushFromTexture(BG, true);
		}

		FSlateBrush Brush = BackgroundImage->GetBrush();
		Brush.ImageSize = FVector2D(W * CellPx, H * CellPx);
		BackgroundImage->SetBrush(Brush);

		BackgroundImage->SetColorAndOpacity(FLinearColor(1,1,1,1));
	}

	if (IconImage)
	{
		if (!ItemRow.Icon.IsNull())
		{
			if (UTexture2D* IconTex = ItemRow.Icon.LoadSynchronous())
			{
				IconImage->SetBrushFromTexture(IconTex, true);

				FSlateBrush Brush = IconImage->GetBrush();
				Brush.ImageSize = FVector2D(W * CellPx, H * CellPx);
				IconImage->SetBrush(Brush);
			}
		}
	}
}