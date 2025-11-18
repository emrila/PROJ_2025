// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomManagerBase.h"
#include "GameFramework/Actor.h"
#include "CombatManager.generated.h"

class AEnemySpawn;

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	Mushroom	UMETA(DisplayName = "Mushroom"),
	Bomber		UMETA(DisplayName = "Bomber"),
	Cactus		UMETA(DisplayName = "Cactus"),
	Golem		UMETA(DisplayName = "Golem")
};

USTRUCT(BlueprintType)
struct FCombatWave
{
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Wave")
	TMap<EEnemyType, int> EnemyCounts;
};

USTRUCT(BlueprintType)
struct FEnemySpawnLocs
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AEnemySpawn*> EnemySpawns;
};
UCLASS()
class PROJ_2025_API ACombatManager : public ARoomManagerBase
{
	GENERATED_BODY()
	
public:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// Sets default values for this actor's properties
	ACombatManager();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_StartWave(int index);

	void StartWave_Internal(int index);

	void RegisterEnemyDeath();
protected:

	virtual void OnRoomInitialized() override;
public:	
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Wave")
	TArray<FCombatWave> Waves;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int RemainingEnemies;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int WaveIndex;
	
private:
	TMap<EEnemyType, TArray<AEnemySpawn*>> EnemyLocations;
	
};
