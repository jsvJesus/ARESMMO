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
enum class EAnimState : uint8
{
	Idle        UMETA(DisplayName="Idle"),
	Walk        UMETA(DisplayName="Walk"),
	Sprint      UMETA(DisplayName="Sprint"),
	CrouchIdle  UMETA(DisplayName="Crouch Idle"),
	CrouchEntry UMETA(DisplayName="Crouch Entry"),
	UnCrouch    UMETA(DisplayName="UnCrouch"),
	CrouchWalk  UMETA(DisplayName="Crouch Walk"),
	Jump        UMETA(DisplayName="Jump"),
	JumpLoop    UMETA(DisplayName="Jump Loop"),
	JumpLand    UMETA(DisplayName="Jump Land")
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