#include "UI/Game/Inventory/InventoryPreviewCaptureActor.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ARESMMO/ARESMMOCharacter.h"

AInventoryPreviewCaptureActor::AInventoryPreviewCaptureActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Capture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("InventoryCapture"));
	Capture->SetupAttachment(Root);

	Capture->bCaptureEveryFrame = true;
	Capture->bCaptureOnMovement = false;
	Capture->FOVAngle = 35.f;
	Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	// сам актёр не должен светиться/мешать в мире
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AInventoryPreviewCaptureActor::Init(AARESMMOCharacter* InCharacter, UTextureRenderTarget2D* InRT)
{
	Character = InCharacter;

	if (!Character.IsValid() || !Capture || !InRT)
	{
		return;
	}

	// ВАЖНО: цепляемся к pivot персонажа, чтобы AddInventoryPreviewYaw продолжал работать
	if (USceneComponent* Pivot = Character->GetInventoryPreviewPivot())
	{
		Capture->AttachToComponent(Pivot, FAttachmentTransformRules::KeepRelativeTransform);
	}

	// как у тебя было на компоненте
	Capture->SetRelativeLocation(FVector(0.f, 150.f, 80.f));
	Capture->SetRelativeRotation(FRotator(0.f, -180.f, 0.f));

	Capture->TextureTarget = InRT;

	// попросим персонажа собрать show-only лист (включая оружие)
	Character->UpdateInventoryPreviewShowOnly();
}