#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/ItemData.h"
#include "WorldItemActor.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;

UCLASS()
class ARESMMO_API AWorldItemActor : public AActor
{
	GENERATED_BODY()

public:
	AWorldItemActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UBoxComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* StaticMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* SkeletalMeshComp;

public:
	/** ID предмета из DataTable (ItemDB::GetItemByID) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	int32 ItemID = 0;

	/** Количество в стаках (простой вариант) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	int32 StackCount = 1;

	/** Кэш строки предмета (заполняется при спавне) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	FItemBaseRow ItemRow;

	virtual void BeginPlay() override;

	/** Инициализация после спавна (по ItemID) */
	UFUNCTION(BlueprintCallable, Category="ARES|Item")
	void InitFromItemID(int32 InItemID, int32 InStackCount = 1);

	/** Вызвать при подборе (например, из BP-интеракции) */
	UFUNCTION(BlueprintCallable, Category="ARES|Item")
	void OnPickedUp(class AARESMMOCharacter* ByCharacter);

protected:
	void LoadItemRow();
	void UpdateVisual();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};