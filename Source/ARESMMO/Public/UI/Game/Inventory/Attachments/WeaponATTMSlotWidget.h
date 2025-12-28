#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "WeaponATTMSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponATTMSlotDetachClicked, EStoreSubCategory, SlotSubCategory);

UCLASS()
class ARESMMO_API UWeaponATTMSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidgetOptional))
	UImage* IconImage = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* NameText = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UButton* DetachButton = nullptr;

	UPROPERTY(BlueprintAssignable, Category="ARES|Weapon|Attachment")
	FOnWeaponATTMSlotDetachClicked OnDetachClicked;

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	void SetSlot(EStoreSubCategory InSlotSubCategory);

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	void SetItem(const FItemBaseRow& ItemRow);

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	void ClearItem();

	UFUNCTION(BlueprintPure, Category="ARES|Weapon|Attachment")
	EStoreSubCategory GetSlotSubCategory() const { return SlotSubCategory; }

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleDetachButton();

private:
	UPROPERTY()
	EStoreSubCategory SlotSubCategory = EStoreSubCategory::None;

	UPROPERTY()
	bool bHasItem = false;
};