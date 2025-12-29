#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EmptySlotWidget.generated.h"

class UImage;

UCLASS()
class ARESMMO_API UEmptySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UImage* SlotImage;

protected:
	virtual void NativeConstruct() override;
};