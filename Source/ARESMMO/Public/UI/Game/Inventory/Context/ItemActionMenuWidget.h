#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/ItemData.h"
#include "ItemActionMenuWidget.generated.h"

class UButton;
class UInventoryWidget;

UENUM(BlueprintType)
enum class EItemContextAction : uint8
{
	Equip          UMETA(DisplayName="Equip"),
	Attach         UMETA(DisplayName="Attach"),
	Use            UMETA(DisplayName="Use"),
	Study          UMETA(DisplayName="Learn"),
	Drop           UMETA(DisplayName="Drop"),
	ChargeItem     UMETA(DisplayName="Charge Item"),
	ChargeMagazine UMETA(DisplayName="Charge Magazine"),
	Repair         UMETA(DisplayName="Repair Item")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemContextAction, EItemContextAction, Action);

UCLASS()
class ARESMMO_API UItemActionMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="ARES|Inventory|ContextMenu")
	FOnItemContextAction OnActionSelected;

	// Настройка меню под конкретный предмет + наличие ресурсов в инвентаре
	UFUNCTION(BlueprintCallable, Category="ARES|Inventory|ContextMenu")
	void SetupForItem(const FItemBaseRow& ItemRow, bool bHasBattery, bool bHasAmmo, bool bHasRepairKit);

	void SetOwnerInventory(UInventoryWidget* InOwner);

protected:
	virtual void NativeConstruct() override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// --- Buttons (BindWidgetOptional чтобы не падало если ты назовёшь по-другому) ---
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Equip = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Attach = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Use = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Study = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Drop = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_ChargeItem = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_ChargeMagazine = nullptr;
	UPROPERTY(meta=(BindWidgetOptional)) UButton* Btn_Repair = nullptr;

	UFUNCTION() void ClickEquip();
	UFUNCTION() void ClickAttach();
	UFUNCTION() void ClickUse();
	UFUNCTION() void ClickStudy();
	UFUNCTION() void ClickDrop();
	UFUNCTION() void ClickChargeItem();
	UFUNCTION() void ClickChargeMagazine();
	UFUNCTION() void ClickRepair();

private:
	void SetBtnVisible(UButton* Btn, bool bVisible);
	void SetBtnEnabled(UButton* Btn, bool bEnabled);
	void Fire(EItemContextAction Action);

	// Context Menu Close
	TWeakObjectPtr<UInventoryWidget> OwnerInventoryWidget;
};