#include "UI/Game/Inventory/EmptySlotWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UEmptySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// ВАЖНО: пустые слоты не должны ловить мышь/дроп
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (SlotImage)
	{
		SlotImage->SetVisibility(ESlateVisibility::HitTestInvisible);

		UTexture2D* Img = LoadObject<UTexture2D>(
			nullptr,
			TEXT("/Game/ARESMMO/UI/Tex/Inventory/ItemSlot/64x64.64x64")
		);

		if (Img)
		{
			SlotImage->SetBrushFromTexture(Img, true);
		}
	}
}