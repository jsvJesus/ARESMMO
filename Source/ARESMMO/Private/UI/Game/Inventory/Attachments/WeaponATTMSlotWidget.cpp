#include "UI/Game/Inventory/Attachments/WeaponATTMSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"

void UWeaponATTMSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DetachButton)
	{
		DetachButton->OnClicked.AddDynamic(this, &UWeaponATTMSlotWidget::HandleDetachButton);
	}
}

void UWeaponATTMSlotWidget::SetSlot(EStoreSubCategory InSlotSubCategory)
{
	SlotSubCategory = InSlotSubCategory;
	ClearItem();
}

void UWeaponATTMSlotWidget::SetItem(const FItemBaseRow& ItemRow)
{
	bHasItem = true;

	if (NameText)
	{
		NameText->SetText(ItemRow.DisplayName.IsEmpty()
			? FText::FromName(ItemRow.InternalName)
			: ItemRow.DisplayName);
		NameText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (IconImage)
	{
		if (!ItemRow.Icon.IsNull())
		{
			if (UTexture2D* Tex = ItemRow.Icon.LoadSynchronous())
			{
				IconImage->SetBrushFromTexture(Tex, true);
				IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
		else
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (DetachButton)
	{
		DetachButton->SetVisibility(ESlateVisibility::Visible);
	}
}

void UWeaponATTMSlotWidget::ClearItem()
{
	bHasItem = false;

	// текст: "Scope: —" и т.п.
	if (NameText)
	{
		const FText SlotName = UEnum::GetDisplayValueAsText(SlotSubCategory);
		NameText->SetText(FText::Format(FText::FromString(TEXT("{0}: —")), SlotName));
		NameText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (IconImage)
	{
		IconImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (DetachButton)
	{
		DetachButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UWeaponATTMSlotWidget::HandleDetachButton()
{
	OnDetachClicked.Broadcast(SlotSubCategory);
}