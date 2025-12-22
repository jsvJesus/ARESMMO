#include "UI/Game/Inventory/EmptySlotWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UEmptySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SlotImage)
	{
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