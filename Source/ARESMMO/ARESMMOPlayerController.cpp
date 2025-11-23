#include "ARESMMOPlayerController.h"
#include "UI/GameHUDWidget.h"
#include "UI/Inventory/InventoryWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Inventory/InventoryComponent.h"
#include "GameFramework/Pawn.h"

AARESMMOPlayerController::AARESMMOPlayerController()
{
	// По умолчанию — чистый режим игры, курсор скрыт
	bShowMouseCursor = false;
}

void AARESMMOPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	CreateGameHUD();
}

void AARESMMOPlayerController::CreateGameHUD()
{
	if (GameHUDInstance || !GameHUDClass)
	{
		return;
	}

	GameHUDInstance = CreateWidget<UGameHUDWidget>(this, GameHUDClass);
	if (GameHUDInstance)
	{
		GameHUDInstance->AddToViewport();

		// Стартовый режим ввода — только игра
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

void AARESMMOPlayerController::ShowInventory()
{
	if (!InventoryWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowInventory: InventoryWidgetClass is not set"));
		return;
	}

	// Создаём виджет один раз
	if (!InventoryWidgetInstance)
	{
		InventoryWidgetInstance = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);

		if (InventoryWidgetInstance)
		{
			// Ищем Pawn → берем его компонент инвентаря
			APawn* MyPawn = GetPawn();
			if (MyPawn)
			{
				if (UInventoryComponent* InvComp = MyPawn->FindComponentByClass<UInventoryComponent>())
				{
					InventoryWidgetInstance->SetInventoryComponent(InvComp);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("ShowInventory: Pawn %s has NO InventoryComponent"), *MyPawn->GetName());
				}
			}
		}
	}
	else
	{
		// На всякий случай пересвязываем, если Pawn сменился
		APawn* MyPawn = GetPawn();
		if (MyPawn)
		{
			if (UInventoryComponent* InvComp = MyPawn->FindComponentByClass<UInventoryComponent>())
			{
				InventoryWidgetInstance->SetInventoryComponent(InvComp);
			}
		}
	}

	// Добавляем виджет на экран
	if (InventoryWidgetInstance && !InventoryWidgetInstance->IsInViewport())
	{
		InventoryWidgetInstance->AddToViewport(10);
	}

	// Настройка управления
	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	bInventoryVisible = true;
}

void AARESMMOPlayerController::HideInventory()
{
	if (InventoryWidgetInstance && InventoryWidgetInstance->IsInViewport())
	{
		InventoryWidgetInstance->RemoveFromParent();
	}

	// Возвращаем чистый игровой ввод
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
	bInventoryVisible = false;
}

void AARESMMOPlayerController::ToggleInventory()
{
	if (bInventoryVisible)
	{
		HideInventory();
	}
	else
	{
		ShowInventory();
	}
}