// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomManagerBase.generated.h"

class ADroppedItem;
class URoomModifierBase;

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Combat      UMETA(DisplayName = "Combat"),
	Parkour    UMETA(DisplayName = "Parkour"),
	Camp        UMETA(DisplayName = "Camp"),
	Event       UMETA(DisplayName = "Event"),
	Boss        UMETA(DisplayName = "Boss"),
};

UCLASS(BlueprintType)
class URoomData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> RoomLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERoomType RoomType;
};

USTRUCT(BlueprintType)
struct FRoomInstance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	URoomData* RoomData = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TArray<TSubclassOf<URoomModifierBase>> ActiveModifierClasses;
};

UCLASS()
class PROJ_2025_API ARoomManagerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ARoomManagerBase();

	virtual void OnRoomInitialized(const FRoomInstance& Room);

	void SpawnLoot();
	
	UFUNCTION()
	virtual void EnableExits();

private:

	UPROPERTY()
	class AUpgradeSpawner* LootSpawnLocation;

	

	UPROPERTY()
	TArray<URoomModifierBase*> RoomModifiers;
	
	
	
};
