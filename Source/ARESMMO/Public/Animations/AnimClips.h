#pragma once

#include "CoreMinimal.h"
#include "AnimClips.generated.h"

USTRUCT()
struct FDirectionalClips
{
	GENERATED_BODY()

	UPROPERTY() UAnimSequence* Forward = nullptr;
	UPROPERTY() UAnimSequence* Backward = nullptr;
	UPROPERTY() UAnimSequence* Left = nullptr;
	UPROPERTY() UAnimSequence* Right = nullptr;
};

USTRUCT()
struct FWeaponAnimationSet
{
	GENERATED_BODY()

	// Idle / Sprint / Jump / etc
	UPROPERTY() UAnimSequence* Idle = nullptr;
	UPROPERTY() UAnimSequence* Sprint = nullptr;
	UPROPERTY() UAnimSequence* CrouchIdle = nullptr;
	UPROPERTY() UAnimSequence* Jump = nullptr;
	UPROPERTY() UAnimSequence* JumpLoop = nullptr;
	UPROPERTY() UAnimSequence* JumpLand = nullptr;

	// Directional movement
	UPROPERTY() FDirectionalClips Walk;
	UPROPERTY() FDirectionalClips CrouchWalk;
};