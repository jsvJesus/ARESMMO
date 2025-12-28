#pragma once

#include "CoreMinimal.h"
#include "ItemSize.generated.h"

/**
 * Размер предмета в инвентаре: Width × Height.
 */
USTRUCT(BlueprintType)
struct FItemSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Width = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Height = 1;

	FItemSize() {}
	FItemSize(int32 W, int32 H) : Width(W), Height(H) {}
};