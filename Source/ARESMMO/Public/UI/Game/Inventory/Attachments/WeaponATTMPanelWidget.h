#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemTypes.h"
#include "WeaponATTMPanelWidget.generated.h"

class AWeaponBase;
class UWeaponATTMSlotWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponATTMDetachRequested, EStoreSubCategory, SlotSubCategory);

UCLASS()
class ARESMMO_API UWeaponATTMPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidgetOptional))
	UWeaponATTMSlotWidget* Slot_Scope = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UWeaponATTMSlotWidget* Slot_Grip = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UWeaponATTMSlotWidget* Slot_Magazine = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UWeaponATTMSlotWidget* Slot_Laser = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UWeaponATTMSlotWidget* Slot_Flashlight = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UWeaponATTMSlotWidget* Slot_Silencer = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UWeaponATTMSlotWidget* Slot_Module = nullptr;

	UPROPERTY(BlueprintAssignable, Category="ARES|Weapon|Attachment")
	FOnWeaponATTMDetachRequested OnDetachRequested;

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	void SetWeapon(AWeaponBase* InWeapon);

	UFUNCTION(BlueprintCallable, Category="ARES|Weapon|Attachment")
	void Refresh();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleDetachClicked(EStoreSubCategory SlotSubCategory);

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AWeaponBase> Weapon;

	void BindSlot(UWeaponATTMSlotWidget* SlotWidget, EStoreSubCategory SubCategory);
	void UpdateSlot(UWeaponATTMSlotWidget* SlotWidget, EStoreSubCategory SubCategory) const;
};