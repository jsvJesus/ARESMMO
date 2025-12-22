#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventoryPreviewCaptureActor.generated.h"

class USceneCaptureComponent2D;
class USceneComponent;
class UTextureRenderTarget2D;
class AARESMMOCharacter;

UCLASS()
class ARESMMO_API AInventoryPreviewCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	AInventoryPreviewCaptureActor();

	void Init(AARESMMOCharacter* InCharacter, UTextureRenderTarget2D* InRT);

	USceneCaptureComponent2D* GetCaptureComponent() const { return Capture; }

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	USceneCaptureComponent2D* Capture = nullptr;

	TWeakObjectPtr<AARESMMOCharacter> Character;
};