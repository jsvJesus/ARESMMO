#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "Items/ItemTypes.h"
#include "EquipmentWidget.generated.h"

class UEquipmentSlotWidget;
class UInventoryLayoutWidget;
class AARESMMOCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentSlotUnequipRequested, EEquipmentSlotType, SlotType);

UCLASS()
class ARESMMO_API UEquipmentWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Конкретные слоты, которые раскидываем по макету вокруг персонажа
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Head;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Body;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Legs;

	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Armor;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Helmet;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Mask;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Backpack;

	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Weapon1;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Weapon2;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Pistol;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Knife;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Grenade;

	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Device1;
	UPROPERTY(meta=(BindWidgetOptional)) UEquipmentSlotWidget* Slot_Device2;

	// Сигнал наружу: "просят снять предмет из этого слота"
	UPROPERTY(BlueprintAssignable, Category="ARES|Equipment")
	FOnEquipmentSlotUnequipRequested OnUnequipRequested;

	void SetOwningLayout(UInventoryLayoutWidget* Layout);

	AARESMMOCharacter* GetPreviewCharacter() const;

protected:
	// Быстрый мап: ENUM → виджет
	UPROPERTY()
	TMap<EEquipmentSlotType, UEquipmentSlotWidget*> SlotMap;

	TWeakObjectPtr<UInventoryLayoutWidget> OwningLayout;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleSlotDoubleClicked(UEquipmentSlotWidget* ClickedSlot);

public:
	// Полностью обновить отображение из мапы персонажа
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void SetEquipment(const TMap<EEquipmentSlotType, FItemBaseRow>& InEquipment);

	// Очистить все слоты (для начала или при Reset)
	UFUNCTION(BlueprintCallable, Category="ARES|Equipment")
	void ClearAllSlots();
};