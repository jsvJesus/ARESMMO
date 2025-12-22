#include "ARESMMOAnimInstance.h"
#include "ARESMMO/ARESMMOCharacter.h"
#include "Weapons/WeaponBase.h"

void UARESMMOAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	APawn* Pawn = TryGetPawnOwner();
	CachedCharacter = Cast<AARESMMOCharacter>(Pawn);
}

void UARESMMOAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	HandsIKWeight = 0.0f;
	HandLTransform = FTransform::Identity;
	HandRTransform = FTransform::Identity;

	AARESMMOCharacter* Char = CachedCharacter.Get();
	if (!Char)
	{
		Char = Cast<AARESMMOCharacter>(TryGetPawnOwner());
		CachedCharacter = Char;
	}
	if (!Char || !Char->GetMesh())
		return;

	// === тут копируем движение всегда ===
	Velocity		= Char->Velocity;
	GroundSpeed		= Char->GroundSpeed;
	Acceleration	= Char->Acceleration;
	ShouldMove		= Char->bShouldMove;
	IsFalling		= Char->bIsFalling;
	DirectionAngle	= Char->DirectionAngle;
	TurnRate		= Char->TurnRate;
	
	Client_WeaponState   = Char->WeaponState;
	Client_MovementState = Char->MoveDirection;
	
	F_OrientationAngle = Char->GetFOrientationAngle();
	R_OrientationAngle = Char->GetROrientationAngle();
	B_OrientationAngle = Char->GetBOrientationAngle();
	L_OrientationAngle = Char->GetLOrientationAngle();

	AWeaponBase* Weapon = Char->GetSelectedWeapon();
	if (!Weapon)
		return;

	FTransform L_World, R_World;
	if (!Weapon->GetHandIKTransforms_World(L_World, R_World))
		return;

	// Выбираем "ведущий" bone, на котором висит оружие
	static const FName Bone_WeaponR(TEXT("weapon_r"));
	static const FName Bone_HandR(TEXT("hand_r"));

	const int32 WeaponRBoneIdx = Char->GetMesh()->GetBoneIndex(Bone_WeaponR);
	const FName DriverBone = (WeaponRBoneIdx != INDEX_NONE) ? Bone_WeaponR : Bone_HandR;

	// Переводим МИРОВУЮ цель левой руки в Bone Space правой руки
	FVector EffLoc_Bone;
	FRotator EffRot_Bone;
	Char->GetMesh()->TransformToBoneSpace(
		DriverBone,
		L_World.GetLocation(),
		L_World.Rotator(),
		EffLoc_Bone,
		EffRot_Bone
	);

	HandLTransform = FTransform(EffRot_Bone, EffLoc_Bone);
	HandsIKWeight = 1.0f;

	// Правую руку IK НЕ двигаем (иначе петля)
	HandRTransform = FTransform::Identity;
}