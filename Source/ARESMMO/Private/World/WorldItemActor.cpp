#include "World/WorldItemActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ARESMMO/ARESMMOCharacter.h"
#include "Items/ItemData.h"
#include "Items/ItemTypes.h"

namespace ItemDB
{
	ARESMMO_API UDataTable* GetItemsDataTable();
	ARESMMO_API const FItemBaseRow* GetItemByID(int32 ItemID);
}

AWorldItemActor::AWorldItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComp->SetupAttachment(Root);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComp->SetupAttachment(Root);

	// Включаем коллизию и оверлап с Pawn
	SetActorEnableCollision(true);

	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	StaticMeshComp->SetGenerateOverlapEvents(true);
	StaticMeshComp->SetCollisionObjectType(ECC_WorldDynamic);
	StaticMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	StaticMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComp->SetGenerateOverlapEvents(false);
}

void AWorldItemActor::BeginPlay()
{
	Super::BeginPlay();

	LoadItemRow();
	UpdateVisual();
}

void AWorldItemActor::InitFromItemID(int32 InItemID, int32 InStackCount)
{
	ItemID      = InItemID;
	StackCount  = FMath::Max(1, InStackCount);

	LoadItemRow();
	UpdateVisual();
}

void AWorldItemActor::LoadItemRow()
{
	if (ItemID <= 0)
		return;

	const FItemBaseRow* Row = ItemDB::GetItemByID(ItemID);
	if (!Row)
		return;

	ItemRow = *Row;
}

void AWorldItemActor::UpdateVisual()
{
	if (!ItemRow.PreviewStaticMesh && !ItemRow.PreviewMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("WorldItemActor: No preview mesh for item %s"), 
			*ItemRow.InternalName.ToString());
		return;
	}

	// StaticMesh вариант
	if (ItemRow.PreviewStaticMesh)
	{
		StaticMeshComp->SetStaticMesh(ItemRow.PreviewStaticMesh);
		StaticMeshComp->SetVisibility(true, true);

		// скрываем skeletal
		SkeletalMeshComp->SetVisibility(false, true);
		SkeletalMeshComp->SetSkeletalMesh(nullptr);
	}

	// SkeletalMesh вариант
	if (ItemRow.PreviewMesh)
	{
		SkeletalMeshComp->SetSkeletalMesh(ItemRow.PreviewMesh);
		SkeletalMeshComp->SetVisibility(true, true);

		// скрываем static
		StaticMeshComp->SetVisibility(false, true);
		StaticMeshComp->SetStaticMesh(nullptr);
	}

	UE_LOG(LogTemp, Log, TEXT("WorldItemActor: Applied preview mesh for item %s"),
		*ItemRow.InternalName.ToString());
}

void AWorldItemActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	UE_LOG(LogTemp, Warning, TEXT("WorldItemActor::NotifyActorBeginOverlap with %s"),
		*GetNameSafe(OtherActor));

	if (!OtherActor)
	{
		return;
	}

	AARESMMOCharacter* Char = Cast<AARESMMOCharacter>(OtherActor);
	if (!Char)
	{
		UE_LOG(LogTemp, Warning, TEXT("Overlap not ARESMMOCharacter"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("WorldItemActor: calling OnPickedUp for %s"),
		*ItemRow.InternalName.ToString());

	OnPickedUp(Char);
}

void AWorldItemActor::OnPickedUp(AARESMMOCharacter* ByCharacter)
{
	if (!ByCharacter)
	{
		return;
	}

	if (ItemID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: invalid ItemID"));
		return;
	}

	const FItemBaseRow* Row = ItemDB::GetItemByID(ItemID);
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: item %d not found in DB"), ItemID);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: trying to add %s to inventory"),
		*Row->InternalName.ToString());

	const bool bAdded = ByCharacter->AddItemToInventory(*Row, StackCount);
	if (bAdded)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: success, destroying actor"));
		Destroy();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnPickedUp: AddItemToInventory failed (no space?)"));
	}
}
