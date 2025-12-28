#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponAttachments.h"
#include "ARESMMO/ARESMMOCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

static bool IsWeaponATTMSubCategory(EStoreSubCategory SubCat)
{
	switch (SubCat)
	{
	case EStoreSubCategory::WeaponATTM_Magazine:
	case EStoreSubCategory::WeaponATTM_Grip:
	case EStoreSubCategory::WeaponATTM_Scope:
	case EStoreSubCategory::WeaponATTM_Laser:
	case EStoreSubCategory::WeaponATTM_Flashlight:
	case EStoreSubCategory::WeaponATTM_Silencer:
	case EStoreSubCategory::WeaponATTM_Module:
		return true;
	default:
		return false;
	}
}

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(Root);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetGenerateOverlapEvents(false);

	MeshGrip = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshGrip"));
	MeshGrip->SetupAttachment(WeaponMesh, GripSocketName);
	MeshGrip->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshGrip->SetGenerateOverlapEvents(false);

	MeshScope = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshScope"));
	MeshScope->SetupAttachment(WeaponMesh, IronSightSocketName);
	MeshScope->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshScope->SetGenerateOverlapEvents(false);

	MeshMuzzle = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshMuzzle"));
	MeshMuzzle->SetupAttachment(WeaponMesh, MuzzleSocketName);
	MeshMuzzle->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshMuzzle->SetGenerateOverlapEvents(false);

	MeshMag = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshMag"));
	MeshMag->SetupAttachment(WeaponMesh, MagSocketName);
	MeshMag->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshMag->SetGenerateOverlapEvents(false);

	MeshLaser = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshLaser"));
	MeshLaser->SetupAttachment(WeaponMesh, LaserSocketName);
	MeshLaser->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshLaser->SetGenerateOverlapEvents(false);

	MeshFlashlight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshFlashlight"));
	MeshFlashlight->SetupAttachment(WeaponMesh, FlashLightSocketName);
	MeshFlashlight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshFlashlight->SetGenerateOverlapEvents(false);

	MeshModule = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshModule"));
	MeshModule->SetupAttachment(WeaponMesh, ModuleSocketName);
	MeshModule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshModule->SetGenerateOverlapEvents(false);

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
			if (!Def.SocketName.IsNone())
			{
				if (AttachmentSubCategory == EStoreSubCategory::WeaponATTM_Scope)
				{
					return GetScopeSocketNameForAttachment(nullptr);
				}
				return Def.SocketName;
			}
			break;
		}
	}

	if (AttachmentSubCategory == EStoreSubCategory::WeaponATTM_Scope)
	{
		return GetScopeSocketNameForAttachment(nullptr);
	}

	return GetDefaultSocketForAttachment(AttachmentSubCategory);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	HideAllAttachmentSlotVisuals();
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

bool AWeaponBase::CheckWeaponATTMClass(const FItemBaseRow& ItemRow, FString& OutFailReason) const
{
	OutFailReason.Empty();

	if (ItemRow.StoreCategory != EStoreCategory::storecat_WeaponATTM)
	{
		OutFailReason = TEXT("Item is not storecat_WeaponATTM");
		return false;
	}

	if (!IsWeaponATTMSubCategory(ItemRow.StoreSubCategory))
	{
		OutFailReason = FString::Printf(TEXT("Invalid WeaponATTM StoreSubCategory: %d"), (int32)ItemRow.StoreSubCategory);
		return false;
	}

	if (!ItemRow.WeaponATTMClass)
	{
		OutFailReason = TEXT("WeaponATTMClass is not set in ItemRow (DataTable).");
		return false;
	}

	return true;
}

UWeaponAttachmentBase* AWeaponBase::CreateAttachmentLogicFromItemRow(const FItemBaseRow& ItemRow,
	FString& OutFailReason) const
{
	OutFailReason.Empty();

	if (!CheckWeaponATTMClass(ItemRow, OutFailReason))
	{
		return nullptr;
	}

	UWeaponAttachmentBase* Logic = NewObject<UWeaponAttachmentBase>(const_cast<AWeaponBase*>(this), ItemRow.WeaponATTMClass);
	if (!Logic)
	{
		OutFailReason = TEXT("Failed to create WeaponATTMClass instance.");
		return nullptr;
	}

	// Защита: класс логики должен соответствовать подкатегории предмета
	if (Logic->HandledSubCategory != EStoreSubCategory::None &&
		Logic->HandledSubCategory != ItemRow.StoreSubCategory)
	{
		OutFailReason = FString::Printf(
			TEXT("WeaponATTMClass HandledSubCategory mismatch. Logic=%d, ItemRow=%d"),
			(int32)Logic->HandledSubCategory,
			(int32)ItemRow.StoreSubCategory
		);
		return nullptr;
	}

	return Logic;
}

bool AWeaponBase::HasAttachment(EStoreSubCategory SubCategory) const
{
	if (const FAttachedWeaponATTM* Found = AttachedATTM.Find(SubCategory))
	{
		return Found->bValid;
	}
	return false;
}

bool AWeaponBase::CanAttachATTMToWeapon(const FItemBaseRow& ItemRow, FString& OutFailReason) const
{
	OutFailReason.Empty();

	// базовая валидность + WeaponATTMClass
	if (!CheckWeaponATTMClass(ItemRow, OutFailReason))
	{
		return false;
	}

	// Запрет 2 одинаковых (по StoreSubCategory)
	if (HasAttachment(ItemRow.StoreSubCategory))
	{
		OutFailReason = TEXT("Attachment of this type is already installed (duplicate).");
		return false;
	}

	// Special: Scope на АК через dovetail (Module)
	if (ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Scope)
	{
		// если есть верхняя планка — ок
		if (bHasTopRail)
		{
			// ok
		}
		else
		{
			// верхней планки нет — прицел можно только через dovetail на АК с установленным Module
			const bool bCanViaDovetail = bIsAKFamilyWeapon && bSupportsDovetailSideMount && bDovetailRailEnabled;
			if (!bCanViaDovetail)
			{
				OutFailReason = TEXT("Scope requires TopRail OR (AK + Dovetail Module installed).");
				return false;
			}
		}
	}

	// Module — только для АК (и только если поддерживает боковую ласточку)
	if (ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Module)
	{
		if (!bIsAKFamilyWeapon || !bSupportsDovetailSideMount)
		{
			OutFailReason = TEXT("Module (dovetail) can be attached only to AK-family weapons that support side mount.");
			return false;
		}
	}

	// Создаём логику и даём ей ещё один шанс (по желанию — дополнительные проверки в классах)
	{
		FString Reason;
		UWeaponAttachmentBase* Logic = CreateAttachmentLogicFromItemRow(ItemRow, Reason);
		if (!Logic)
		{
			OutFailReason = Reason;
			return false;
		}

		if (!Logic->CanAttachToWeapon(this, ItemRow))
		{
			OutFailReason = TEXT("WeaponATTMClass::CanAttachToWeapon returned false.");
			return false;
		}
	}

	return true;
}

bool AWeaponBase::AttachATTMToWeapon(const FItemBaseRow& ItemRow, FString& OutFailReason)
{
	OutFailReason.Empty();

	// общая проверка (включая duplicates и dovetail)
	if (!CanAttachATTMToWeapon(ItemRow, OutFailReason))
	{
		return false;
	}

	// создать логику
	UWeaponAttachmentBase* Logic = CreateAttachmentLogicFromItemRow(ItemRow, OutFailReason);
	if (!Logic)
	{
		return false;
	}

	// выполнить attach
	if (!Logic->AttachToWeapon(this, ItemRow))
	{
		OutFailReason = TEXT("WeaponATTMClass::AttachToWeapon failed.");
		return false;
	}

	// сохранить
	FAttachedWeaponATTM NewSlot;
	NewSlot.Set(ItemRow, Logic);
	AttachedATTM.Add(ItemRow.StoreSubCategory, NewSlot);

	// Module включает dovetail rail
	if (ItemRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Module)
	{
		bDovetailRailEnabled = true;
	}

	return true;
}

bool AWeaponBase::DetachATTMFromWeapon(EStoreSubCategory SubCategory, FItemBaseRow& OutDetachedItemRow,
	FString& OutFailReason)
{
	OutFailReason.Empty();

	FAttachedWeaponATTM* Found = AttachedATTM.Find(SubCategory);
	if (!Found || !Found->bValid)
	{
		OutFailReason = TEXT("No attachment in that slot.");
		return false;
	}

	// Защита: нельзя снять Module, если Scope держится только на dovetail (нет TopRail)
	if (SubCategory == EStoreSubCategory::WeaponATTM_Module)
	{
		const bool bScopeInstalled = HasAttachment(EStoreSubCategory::WeaponATTM_Scope);
		const bool bScopeNeedsModule = bScopeInstalled && !bHasTopRail; // если верхней планки нет, прицел был возможен только через dovetail
		if (bScopeNeedsModule)
		{
			OutFailReason = TEXT("Cannot detach Dovetail Module while a Scope is installed and there is no TopRail. Detach Scope first.");
			return false;
		}
	}

	// вызвать Detach у логики
	if (Found->Logic && !Found->Logic->DetachFromWeapon(this))
	{
		OutFailReason = TEXT("WeaponATTMClass::DetachFromWeapon failed.");
		return false;
	}

	OutDetachedItemRow = Found->ItemRow;

	// убрать из слота
	AttachedATTM.Remove(SubCategory);

	// выключить dovetail rail если сняли Module
	if (SubCategory == EStoreSubCategory::WeaponATTM_Module)
	{
		bDovetailRailEnabled = false;
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
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: WeaponMesh is null."));
		return false;
	}

	// базовые проверки + WeaponATTMClass
	FString FailReason;
	if (!CheckWeaponATTMClass(AttachmentRow, FailReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: %s | Item=%s"),
			*FailReason, *AttachmentRow.InternalName.ToString());
		return false;
	}

	// должен быть слот/сокет под эту подкатегорию
	if (!CanAcceptAttachment(AttachmentRow))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: Weapon has no socket for this attachment type. | Item=%s Sub=%d"),
			*AttachmentRow.InternalName.ToString(), (int32)AttachmentRow.StoreSubCategory);
		return false;
	}

	// основная ATTM-логика
	if (!AttachATTMToWeapon(AttachmentRow, FailReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: AttachATTMToWeapon failed: %s | Item=%s"),
			*FailReason, *AttachmentRow.InternalName.ToString());
		return false;
	}

	// визуал (меш) — опционально (Module может быть без меша)
	const FName Socket = (AttachmentRow.StoreSubCategory == EStoreSubCategory::WeaponATTM_Scope)
		? GetScopeSocketNameForAttachment(&AttachmentRow)
		: GetSocketForAttachment(AttachmentRow.StoreSubCategory);

	// на всякий случай (если вдруг визуал остался)
	DetachAttachment(AttachmentRow.StoreSubCategory);

	if (UStaticMeshComponent* SlotMesh = GetAttachmentMeshComponent(AttachmentRow.StoreSubCategory))
	{
		UStaticMesh* NewMesh = AttachmentRow.WeaponAttachmentStaticMesh;

		SlotMesh->SetStaticMesh(NewMesh);

		SlotMesh->AttachToComponent(
			WeaponMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			Socket
		);

		// Показываем ТОЛЬКО если реально есть меш (если модуль “логический” без меша — останется скрытым)
		SetSlotVisualVisible(SlotMesh, NewMesh != nullptr);
	}
	else if (AttachmentRow.WeaponAttachmentSkeletalMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttachItem: Skeletal mesh attachments are not supported in static slots. | Item=%s"),
			*AttachmentRow.InternalName.ToString());
		FItemBaseRow DummyRow;
		FString DetachReason;
		DetachATTMFromWeapon(AttachmentRow.StoreSubCategory, DummyRow, DetachReason);
		return false;
	}

	return true;
}

bool AWeaponBase::DetachItem(EStoreSubCategory SubCategory, FItemBaseRow& OutDetachedItemRow, FString& OutFailReason)
{
	OutFailReason.Empty();

	// снять логику (проверка “нельзя снять Module если Scope держится на dovetail” — внутри)
	if (!DetachATTMFromWeapon(SubCategory, OutDetachedItemRow, OutFailReason))
	{
		return false;
	}

	// снять визуал
	DetachAttachment(SubCategory);

	return true;
}

bool AWeaponBase::DetachAttachment(EStoreSubCategory AttachmentSubCategory)
{
	if (UStaticMeshComponent* SlotMesh = GetAttachmentMeshComponent(AttachmentSubCategory))
	{
		SlotMesh->SetStaticMesh(nullptr);
		SetSlotVisualVisible(SlotMesh, false);

		// Вернём на “дефолтный” сокет (для Scope уйдёт на IronSight)
		if (WeaponMesh)
		{
			const FName Sock = GetSocketForAttachment(AttachmentSubCategory);
			if (!Sock.IsNone())
			{
				SlotMesh->AttachToComponent(
					WeaponMesh,
					FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					Sock
				);
			}
		}

		if (AttachmentSubCategory == EStoreSubCategory::WeaponATTM_Scope)
		{
			ResetScopeSocketIfNeeded();
		}
		return true;
	}
	return false;
}

UStaticMeshComponent* AWeaponBase::GetAttachmentMeshComponent(EStoreSubCategory SubCategory) const
{
	switch (SubCategory)
	{
	case EStoreSubCategory::WeaponATTM_Grip:
		return MeshGrip;
	case EStoreSubCategory::WeaponATTM_Scope:
		return MeshScope;
	case EStoreSubCategory::WeaponATTM_Magazine:
		return MeshMag;
	case EStoreSubCategory::WeaponATTM_Laser:
		return MeshLaser;
	case EStoreSubCategory::WeaponATTM_Flashlight:
		return MeshFlashlight;
	case EStoreSubCategory::WeaponATTM_Silencer:
		return MeshMuzzle;
	case EStoreSubCategory::WeaponATTM_Module:
		return MeshModule;
	default:
		return nullptr;
	}
}

FName AWeaponBase::GetDefaultSocketForAttachment(EStoreSubCategory AttachmentSubCategory) const
{
	switch (AttachmentSubCategory)
	{
	case EStoreSubCategory::WeaponATTM_Grip:
		return GripSocketName;
	case EStoreSubCategory::WeaponATTM_Scope:
		return ScopeSocketName;
	case EStoreSubCategory::WeaponATTM_Magazine:
		return MagSocketName;
	case EStoreSubCategory::WeaponATTM_Laser:
		return LaserSocketName;
	case EStoreSubCategory::WeaponATTM_Flashlight:
		return FlashLightSocketName;
	case EStoreSubCategory::WeaponATTM_Silencer:
		return MuzzleSocketName;
	case EStoreSubCategory::WeaponATTM_Module:
		return ModuleSocketName;
	default:
		return NAME_None;
	}
}

FName AWeaponBase::GetScopeSocketNameForAttachment(const FItemBaseRow* AttachmentRow) const
{
	if (AttachmentRow && AttachmentRow->WeaponAttachmentStaticMesh)
	{
		return ScopeSocketName;
	}

	if (MeshScope && MeshScope->GetStaticMesh())
	{
		return ScopeSocketName;
	}

	return IronSightSocketName;
}

void AWeaponBase::ResetScopeSocketIfNeeded()
{
	if (MeshScope && WeaponMesh)
	{
		const FName Socket = GetScopeSocketNameForAttachment(nullptr);
		MeshScope->AttachToComponent(WeaponMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
	}
}

void AWeaponBase::SetSlotVisualVisible(UStaticMeshComponent* Comp, bool bVisible) const
{
	if (!Comp) return;

	Comp->SetHiddenInGame(!bVisible, true);
	Comp->SetVisibility(bVisible, true);
}

void AWeaponBase::HideAllAttachmentSlotVisuals()
{
	auto HideSlot = [&](UStaticMeshComponent* Comp, EStoreSubCategory SubCat)
	{
		if (!Comp) return;

		Comp->SetStaticMesh(nullptr);

		// Приводим компонент к “дефолтному” сокету
		if (WeaponMesh)
		{
			const FName Sock = GetSocketForAttachment(SubCat);
			if (!Sock.IsNone())
			{
				Comp->AttachToComponent(
					WeaponMesh,
					FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					Sock
				);
			}
		}

		SetSlotVisualVisible(Comp, false);
	};

	HideSlot(MeshGrip,       EStoreSubCategory::WeaponATTM_Grip);
	HideSlot(MeshScope,      EStoreSubCategory::WeaponATTM_Scope);
	HideSlot(MeshMuzzle,     EStoreSubCategory::WeaponATTM_Silencer);
	HideSlot(MeshMag,        EStoreSubCategory::WeaponATTM_Magazine);
	HideSlot(MeshLaser,      EStoreSubCategory::WeaponATTM_Laser);
	HideSlot(MeshFlashlight, EStoreSubCategory::WeaponATTM_Flashlight);
	HideSlot(MeshModule,     EStoreSubCategory::WeaponATTM_Module);
}