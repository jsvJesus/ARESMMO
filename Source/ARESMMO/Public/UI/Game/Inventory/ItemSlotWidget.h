#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "ItemSlotWidget.generated.h"

class UImage;

UCLASS()
class ARESMMO_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Фон предмета (64x64 / 128x128 / 128x256 / 192x192 / 256x128 и т.д.) */
	UPROPERTY(meta=(BindWidget))
	UImage* BackgroundImage;

	/** Иконка предмета */
	UPROPERTY(meta=(BindWidget))
	UImage* IconImage;

	/** Устанавливается из InventoryWidget::CreateItemWidget */
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory")
	void InitItem(const FItemBaseRow& ItemRow, const FItemSize& GridSize);

protected:
	virtual void NativeConstruct() override;

private:
	// Выбор правильного фона по размеру предмета
	UTexture2D* GetBackgroundTexture(const FItemSize& GridSize) const;
};