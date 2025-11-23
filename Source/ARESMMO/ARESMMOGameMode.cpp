// Copyright Epic Games, Inc. All Rights Reserved.

#include "ARESMMOGameMode.h"
#include "ARESMMOCharacter.h"
#include "ARESMMOPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AARESMMOGameMode::AARESMMOGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PlayerControllerClass = AARESMMOPlayerController::StaticClass();
}
