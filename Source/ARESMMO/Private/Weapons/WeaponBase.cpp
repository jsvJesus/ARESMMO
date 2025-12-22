#include "Weapons/WeaponBase.h"
#include "ARESMMO/ARESMMOCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetGenerateOverlapEvents(false);

	// На будущее (сеть)
	bReplicates = true;
	SetReplicateMovement(false);
}

void AWeaponBase::SetOwningCharacter(AARESMMOCharacter* NewOwner)
{
	OwningCharacter = NewOwner;
	SetOwner(NewOwner);
}

void AWeaponBase::Fire()
{
	UE_LOG(LogTemp, Log, TEXT("Weapon Fire: %s"), *GetName());
}

void AWeaponBase::Reload()
{
	UE_LOG(LogTemp, Log, TEXT("Weapon Reload: %s"), *GetName());
}

void AWeaponBase::StartAim()
{
	UE_LOG(LogTemp, Log, TEXT("Weapon Aim Start: %s"), *GetName());
}

void AWeaponBase::StopAim()
{
	UE_LOG(LogTemp, Log, TEXT("Weapon Aim Stop: %s"), *GetName());
}

FName AWeaponBase::GetSocketForAttachment(EStoreSubCategory AttachmentSubCategory) const
{
	for (const FWeaponAttachmentSlotDef& Def : AttachmentSlots)
	{
		if (Def.AttachmentSubCategory == AttachmentSubCategory)
		{
			return Def.SocketName;
		}
	}
	return NAME_None;
}

bool AWeaponBase::GetHandIKTransforms_World(FTransform& OutL, FTransform& OutR) const
{
	OutL = FTransform::Identity;
	OutR = FTransform::Identity;

	if (!OwningCharacter || !OwningCharacter->GetMesh())
		return false;

	USkeletalMeshComponent* CharMesh = OwningCharacter->GetMesh();

	const bool bHasL = CharMesh->DoesSocketExist(LeftHandIKSocket);
	const bool bHasR = CharMesh->DoesSocketExist(RightHandIKSocket);

	if (!bHasL)
	{
		// Не ломаем анимацию: просто вернём false и IK вес останется 0
		UE_LOG(LogTemp, Warning,
			TEXT("Weapon %s: missing LEFT IK socket on CHARACTER (%s). CharacterMesh=%s"),
			*GetName(),
			*LeftHandIKSocket.ToString(),
			*GetNameSafe(CharMesh->GetSkeletalMeshAsset())
		);
		return false;
	}

	OutL = CharMesh->GetSocketTransform(LeftHandIKSocket, RTS_World);

	// Right socket не обязателен
	if (bHasR)
	{
		OutR = CharMesh->GetSocketTransform(RightHandIKSocket, RTS_World);
	}

	return true;
}

bool AWeaponBase::CanAcceptAttachment(const FItemBaseRow& AttachmentRow) const
{
	if (AttachmentRow.StoreCategory != EStoreCategory::storecat_WeaponATTM)
		return false;

	const FName Socket = GetSocketForAttachment(AttachmentRow.StoreSubCategory);
	if (Socket.IsNone())
		return false;

	// Можно расширить правилами (совместимость, список допустимых, калибр и т.д.)
	return true;
}

bool AWeaponBase::AttachItem(const FItemBaseRow& AttachmentRow)
{
	if (!WeaponMesh)
		return false;

	if (!CanAcceptAttachment(AttachmentRow))
		return false;

	const FName Socket = GetSocketForAttachment(AttachmentRow.StoreSubCategory);
	if (Socket.IsNone())
		return false;

	// Если в этом слоте уже что-то есть — снимаем
	DetachAttachment(AttachmentRow.StoreSubCategory);

	USceneComponent* NewComp = nullptr;

	if (AttachmentRow.WeaponAttachmentStaticMesh)
	{
		UStaticMeshComponent* SM = NewObject<UStaticMeshComponent>(this);
		SM->SetStaticMesh(AttachmentRow.WeaponAttachmentStaticMesh);
		SM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SM->SetGenerateOverlapEvents(false);
		NewComp = SM;
	}
	else if (AttachmentRow.WeaponAttachmentSkeletalMesh)
	{
		USkeletalMeshComponent* SK = NewObject<USkeletalMeshComponent>(this);
		SK->SetSkeletalMesh(AttachmentRow.WeaponAttachmentSkeletalMesh);
		SK->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SK->SetGenerateOverlapEvents(false);
		NewComp = SK;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: no attachment mesh in DT for %s"), *AttachmentRow.InternalName.ToString());
		return false;
	}

	if (!NewComp)
		return false;

	NewComp->RegisterComponent();
	NewComp->AttachToComponent(WeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);

	RuntimeAttachments.Add(AttachmentRow.StoreSubCategory, NewComp);

	UE_LOG(LogTemp, Log, TEXT("Attached %s to %s socket=%s"),
		*AttachmentRow.InternalName.ToString(), *GetName(), *Socket.ToString());

	return true;
}

bool AWeaponBase::DetachAttachment(EStoreSubCategory AttachmentSubCategory)
{
	if (TObjectPtr<USceneComponent>* Found = RuntimeAttachments.Find(AttachmentSubCategory))
	{
		if (USceneComponent* Comp = *Found)
		{
			Comp->DestroyComponent();
		}
		RuntimeAttachments.Remove(AttachmentSubCategory);
		return true;
	}
	return false;
}