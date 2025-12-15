#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "ItemDragDropWidget.generated.h"

class UImage;

UCLASS()
class ARESMMO_API UItemDragDropWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UImage* BackgroundImage = nullptr;

	UPROPERTY(meta=(BindWidget))
	UImage* IconImage = nullptr;

	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|DragDrop")
	void InitDragVisual(const FItemBaseRow& ItemRow, const FItemSize& SizeInCells, float CellPx = 64.f);

protected:
	virtual void NativeConstruct() override;

private:
	UTexture2D* GetBackgroundTexture(const FItemSize& GridSize) const;
};