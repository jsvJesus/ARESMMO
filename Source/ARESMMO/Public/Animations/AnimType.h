#pragma once

#include "CoreMinimal.h"
#include "AnimType.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Unarmed    UMETA(DisplayName="Unarmed"),
	Rifle      UMETA(DisplayName="Rifle"),
	Shotgun    UMETA(DisplayName="Shotgun"),
	Pistol     UMETA(DisplayName="Pistol"),
	Melee      UMETA(DisplayName="Melee"),
	Grenade    UMETA(DisplayName="Grenade"),
	PlaceItem  UMETA(DisplayName="Place Item"),
	UsableItem UMETA(DisplayName="Usable Item")
};

UENUM(BlueprintType)
enum class EMoveDirection : uint8
{
	None     UMETA(DisplayName="None"),
	Forward  UMETA(DisplayName="Forward"),
	Backward UMETA(DisplayName="Backward"),
	Left     UMETA(DisplayName="Left"),
	Right    UMETA(DisplayName="Right")
};

UENUM(BlueprintType)
enum class ELandState : uint8
{
	None  UMETA(DisplayName="None"),
	Normal  UMETA(DisplayName="Normal"),
	Soft  UMETA(DisplayName="Soft"),
	Hard  UMETA(DisplayName="Hard")
};