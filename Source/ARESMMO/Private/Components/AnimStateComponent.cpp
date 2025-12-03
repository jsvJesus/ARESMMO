#include "Components/AnimStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "UObject/ConstructorHelpers.h"

UAnimStateComponent::UAnimStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ====== UNARMED ======
	{
		FWeaponAnimationSet Unarmed;

		// Idle
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleObj(
				TEXT("/Game/ARESMMO/Animation/MM_Unarmed_Idle_Ready.MM_Unarmed_Idle_Ready")
			);
			if (IdleObj.Succeeded())
			{
				Unarmed.Idle = IdleObj.Object;
			}
		}

		// Walk (Forward / Left / Right / Backward)
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkFObj(
				TEXT("/Game/ARESMMO/Animation/MM_Unarmed_Walk_Fwd.MM_Unarmed_Walk_Fwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkBObj(
				TEXT("/Game/ARESMMO/Animation/MM_Unarmed_Walk_Bwd.MM_Unarmed_Walk_Bwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkLObj(
				TEXT("/Game/ARESMMO/Animation/MM_Unarmed_Walk_Left.MM_Unarmed_Walk_Left")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkRObj(
				TEXT("/Game/ARESMMO/Animation/MM_Unarmed_Walk_Right.MM_Unarmed_Walk_Right")
			);

			if (WalkFObj.Succeeded()) Unarmed.Walk.Forward  = WalkFObj.Object;
			if (WalkBObj.Succeeded()) Unarmed.Walk.Backward = WalkBObj.Object;
			if (WalkLObj.Succeeded()) Unarmed.Walk.Left     = WalkLObj.Object;
			if (WalkRObj.Succeeded()) Unarmed.Walk.Right    = WalkRObj.Object;
		}

		// Sprint (прямо)
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> SprintObj(
				TEXT("/Game/ARESMMO/Animation/Jog/MM_Unarmed_Jog_Fwd.MM_Unarmed_Jog_Fwd")
			);
			if (SprintObj.Succeeded())
			{
				Unarmed.Sprint = SprintObj.Object;
			}
		}

		// Crouch Idle
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CrouchIdleObj(
				TEXT("/Game/ARESMMO/Animation/Crouch/MM_Unarmed_Crouch_Idle.MM_Unarmed_Crouch_Idle")
			);
			if (CrouchIdleObj.Succeeded())
			{
				Unarmed.CrouchIdle = CrouchIdleObj.Object;
			}
		}

		// Crouch Walk
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_F(
				TEXT("/Game/ARESMMO/Animation/Crouch/MM_Unarmed_Crouch_Walk_Fwd.MM_Unarmed_Crouch_Walk_Fwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_B(
				TEXT("/Game/ARESMMO/Animation/Crouch/MM_Unarmed_Crouch_Walk_Bwd.MM_Unarmed_Crouch_Walk_Bwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_L(
				TEXT("/Game/ARESMMO/Animation/Crouch/MM_Unarmed_Crouch_Walk_Left.MM_Unarmed_Crouch_Walk_Left")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_R(
				TEXT("/Game/ARESMMO/Animation/Crouch/MM_Unarmed_Crouch_Walk_Right.MM_Unarmed_Crouch_Walk_Right")
			);

			if (CW_F.Succeeded()) Unarmed.CrouchWalk.Forward  = CW_F.Object;
			if (CW_B.Succeeded()) Unarmed.CrouchWalk.Backward = CW_B.Object;
			if (CW_L.Succeeded()) Unarmed.CrouchWalk.Left     = CW_L.Object;
			if (CW_R.Succeeded()) Unarmed.CrouchWalk.Right    = CW_R.Object;
		}

		AnimConfig.Add(EWeaponState::Unarmed, Unarmed);
	}

	// ====== RIFLE ======
	{
		FWeaponAnimationSet Rifle;

		// Idle
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleObj(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Idle_ADS.MM_Rifle_Idle_ADS")
			);
			if (IdleObj.Succeeded())
			{
				Rifle.Idle = IdleObj.Object;
			}
		}

		// Walk (F/L/R/B)
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkFObj(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Walk_Fwd.MM_Rifle_Walk_Fwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkBObj(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Walk_Bwd.MM_Rifle_Walk_Bwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkLObj(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Walk_Left.MM_Rifle_Walk_Left")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkRObj(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Walk_Right.MM_Rifle_Walk_Right")
			);

			if (WalkFObj.Succeeded()) Rifle.Walk.Forward  = WalkFObj.Object;
			if (WalkBObj.Succeeded()) Rifle.Walk.Backward = WalkBObj.Object;
			if (WalkLObj.Succeeded()) Rifle.Walk.Left     = WalkLObj.Object;
			if (WalkRObj.Succeeded()) Rifle.Walk.Right    = WalkRObj.Object;
		}

		// Sprint
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> SprintObj(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Jog_Fwd.MM_Rifle_Jog_Fwd")
			);
			if (SprintObj.Succeeded())
			{
				Rifle.Sprint = SprintObj.Object;
			}
		}

		// Crouch Idle
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CrouchIdleObj(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Crouch_Idle.MM_Rifle_Crouch_Idle")
			);
			if (CrouchIdleObj.Succeeded())
			{
				Rifle.CrouchIdle = CrouchIdleObj.Object;
			}
		}

		// Crouch Walk
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_F(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Crouch_Walk_Fwd.MM_Rifle_Crouch_Walk_Fwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_B(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Crouch_Walk_Bwd.MM_Rifle_Crouch_Walk_Bwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_L(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Crouch_Walk_Left.MM_Rifle_Crouch_Walk_Left")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_R(
				TEXT("/Game/ARESMMO/Animation/Rifle/MM_Rifle_Crouch_Walk_Right.MM_Rifle_Crouch_Walk_Right")
			);

			if (CW_F.Succeeded()) Rifle.CrouchWalk.Forward  = CW_F.Object;
			if (CW_B.Succeeded()) Rifle.CrouchWalk.Backward = CW_B.Object;
			if (CW_L.Succeeded()) Rifle.CrouchWalk.Left     = CW_L.Object;
			if (CW_R.Succeeded()) Rifle.CrouchWalk.Right    = CW_R.Object;
		}

		AnimConfig.Add(EWeaponState::Rifle, Rifle);
	}

	// ====== PISTOL ======
	{
		FWeaponAnimationSet Pistol;

		// Idle
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> IdleObj(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Idle_ADS.MM_Pistol_Idle_ADS")
			);
			if (IdleObj.Succeeded())
			{
				Pistol.Idle = IdleObj.Object;
			}
		}

		// Walk (F/L/R/B)
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkFObj(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Walk_Fwd.MM_Pistol_Walk_Fwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkBObj(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Walk_Bwd.MM_Pistol_Walk_Bwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkLObj(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Walk_Left.MM_Pistol_Walk_Left")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> WalkRObj(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Walk_Right.MM_Pistol_Walk_Right")
			);

			if (WalkFObj.Succeeded()) Pistol.Walk.Forward  = WalkFObj.Object;
			if (WalkBObj.Succeeded()) Pistol.Walk.Backward = WalkBObj.Object;
			if (WalkLObj.Succeeded()) Pistol.Walk.Left     = WalkLObj.Object;
			if (WalkRObj.Succeeded()) Pistol.Walk.Right    = WalkRObj.Object;
		}

		// Sprint
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> SprintObj(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Jog_Fwd.MM_Pistol_Jog_Fwd")
			);
			if (SprintObj.Succeeded())
			{
				Pistol.Sprint = SprintObj.Object;
			}
		}

		// Crouch Idle
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CrouchIdleObj(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Crouch_Idle.MM_Pistol_Crouch_Idle")
			);
			if (CrouchIdleObj.Succeeded())
			{
				Pistol.CrouchIdle = CrouchIdleObj.Object;
			}
		}

		// Crouch Walk
		{
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_F(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Crouch_Walk_Fwd.MM_Pistol_Crouch_Walk_Fwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_B(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Crouch_Walk_Bwd.MM_Pistol_Crouch_Walk_Bwd")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_L(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Crouch_Walk_Left.MM_Pistol_Crouch_Walk_Left")
			);
			static ConstructorHelpers::FObjectFinder<UAnimSequence> CW_R(
				TEXT("/Game/ARESMMO/Animation/Pistol/MM_Pistol_Crouch_Walk_Right.MM_Pistol_Crouch_Walk_Right")
			);

			if (CW_F.Succeeded()) Pistol.CrouchWalk.Forward  = CW_F.Object;
			if (CW_B.Succeeded()) Pistol.CrouchWalk.Backward = CW_B.Object;
			if (CW_L.Succeeded()) Pistol.CrouchWalk.Left     = CW_L.Object;
			if (CW_R.Succeeded()) Pistol.CrouchWalk.Right    = CW_R.Object;
		}

		AnimConfig.Add(EWeaponState::Pistol, Pistol);
	}
}

void UAnimStateComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter.IsValid())
	{
		MoveComp = OwnerCharacter->GetCharacterMovement();
		Mesh     = OwnerCharacter->GetMesh();

		if (Mesh.IsValid())
		{
			Mesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		}
	}

	// начальное состояние: без оружия, стоим
	UpdateAnimation();
}

void UAnimStateComponent::SetWeaponState(EWeaponState NewState)
{
	if (WeaponState == NewState)
	{
		return;
	}
	WeaponState = NewState;
	UpdateAnimation();
}

void UAnimStateComponent::UpdateMoveInput(const FVector2D& MoveInput)
{
	LastMoveInput = MoveInput;
	RecalculateDirection();
	UpdateAnimation();
}

void UAnimStateComponent::UpdateMovementFlags(bool bInSprint, bool bInCrouch, bool bInAir, float Speed)
{
	RecalculateAnimState(bInSprint, bInCrouch, bInAir, Speed);
	UpdateAnimation();
}

void UAnimStateComponent::RecalculateAnimState(bool bInSprint, bool bInCrouch, bool bInAir, float Speed)
{
	// Простая дискретная логика. При желании можно расширить (Jump / JumpLoop / JumpLand по эвентам).
	if (bInAir)
	{
		// Пока нет отдельных эвентов — держим в JumpLoop
		AnimState = EAnimState::JumpLoop;
		return;
	}

	const bool bHasMoveInput = !LastMoveInput.IsNearlyZero(0.1f);

	if (bInCrouch)
	{
		if (bHasMoveInput)
		{
			AnimState = EAnimState::CrouchWalk;
		}
		else
		{
			AnimState = EAnimState::CrouchIdle;
		}
		return;
	}

	if (bInSprint && bHasMoveInput)
	{
		AnimState = EAnimState::Sprint;
		return;
	}

	if (bHasMoveInput)
	{
		AnimState = EAnimState::Walk;
	}
	else
	{
		AnimState = EAnimState::Idle;
	}
}

void UAnimStateComponent::RecalculateDirection()
{
	// MoveInput.Y = Forward, MoveInput.X = Right)
	if (LastMoveInput.IsNearlyZero(0.1f))
	{
		MoveDirection = EMoveDirection::None;
		return;
	}

	const float Forward = LastMoveInput.Y;
	const float Right   = LastMoveInput.X;

	if (FMath::Abs(Forward) >= FMath::Abs(Right))
	{
		MoveDirection = (Forward >= 0.f) ? EMoveDirection::Forward : EMoveDirection::Backward;
	}
	else
	{
		MoveDirection = (Right >= 0.f) ? EMoveDirection::Right : EMoveDirection::Left;
	}
}

UAnimSequence* UAnimStateComponent::SelectCurrentClip() const
{
	const FWeaponAnimationSet* SetPtr = AnimConfig.Find(WeaponState);
	if (!SetPtr)
	{
		return nullptr;
	}

	const FWeaponAnimationSet& Set = *SetPtr;
	UAnimSequence* Clip = nullptr;

	switch (AnimState)
	{
	case EAnimState::Idle:
		Clip = Set.Idle;
		break;

	case EAnimState::Sprint:
		Clip = Set.Sprint;
		break;

	case EAnimState::CrouchIdle:
		Clip = Set.CrouchIdle;
		break;

	case EAnimState::Walk:
		switch (MoveDirection)
		{
	case EMoveDirection::Forward:  Clip = Set.Walk.Forward;  break;
	case EMoveDirection::Backward: Clip = Set.Walk.Backward; break;
	case EMoveDirection::Left:     Clip = Set.Walk.Left;     break;
	case EMoveDirection::Right:    Clip = Set.Walk.Right;    break;
	default:                       Clip = Set.Walk.Forward;  break;
		}
		break;

	case EAnimState::CrouchWalk:
		switch (MoveDirection)
		{
	case EMoveDirection::Forward:  Clip = Set.CrouchWalk.Forward;  break;
	case EMoveDirection::Backward: Clip = Set.CrouchWalk.Backward; break;
	case EMoveDirection::Left:     Clip = Set.CrouchWalk.Left;     break;
	case EMoveDirection::Right:    Clip = Set.CrouchWalk.Right;    break;
	default:                       Clip = Set.CrouchWalk.Forward;  break;
		}
		break;

	case EAnimState::Jump:
		Clip = Set.Jump;
		break;

	case EAnimState::JumpLoop:
		Clip = Set.JumpLoop;
		break;

	case EAnimState::JumpLand:
		Clip = Set.JumpLand;
		break;

	default:
		break;
	}

	return Clip;
}

void UAnimStateComponent::UpdateAnimation()
{
	if (!Mesh.IsValid())
	{
		return;
	}

	if (UAnimSequence* Clip = SelectCurrentClip())
	{
		Mesh->PlayAnimation(Clip, true); // true = loop
	}
}