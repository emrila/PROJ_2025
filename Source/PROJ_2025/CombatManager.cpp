// Furkan approves of this


#include "CombatManager.h"

#include "BomberCharacter.h"
#include "EnemySpawn.h"
#include "MushroomCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ACombatManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ACombatManager, RemainingEnemies);
	DOREPLIFETIME(ACombatManager, WaveIndex);
}

// Sets default values
ACombatManager::ACombatManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ACombatManager::Server_StartWave_Implementation(int index)
{
	StartWave_Internal(index);
}


void ACombatManager::StartWave_Internal(int index)
{
	if (!Waves.IsValidIndex(index))
	{
		SpawnLoot();
		return;
	};
	UE_LOG(LogTemp, Display, TEXT("Wave: %d"), index);
	RemainingEnemies = Waves[index].Enemies.Num();
	for (AEnemySpawn* SpawnPoint : Waves[index].Enemies)
	{
		if (SpawnPoint && SpawnPoint->EnemyClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(
				SpawnPoint->EnemyClass,
				SpawnPoint->GetActorLocation(),
				SpawnPoint->GetActorRotation()
			);
			if (AEnemyBase* Enemy = Cast<AEnemyBase>(SpawnedActor))
			{
				Enemy->CombatManager = this;
			}
		}
	}
}

void ACombatManager::RegisterEnemyDeath()
{
	if (!HasAuthority()) return;
	
	RemainingEnemies--;
	if (RemainingEnemies == 0)
	{
		WaveIndex++;
		Server_StartWave(WaveIndex);
	}
}

void ACombatManager::OnRoomInitialized()
{
	Super::OnRoomInitialized();
	TArray<AActor*> FoundSpawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawn::StaticClass(), FoundSpawns);

	TMap<int32, TArray<AEnemySpawn*>> WaveMap;
	Waves.Empty();
	WaveIndex = 0;
	for (AActor* Actor : FoundSpawns)
	{
		if (AEnemySpawn* Spawn = Cast<AEnemySpawn>(Actor))
		{
			WaveMap.FindOrAdd(Spawn->WaveNumber).Add(Spawn);
		}
	}

	WaveMap.KeySort([](int32 A, int32 B) { return A < B; });

	for (auto& Pair : WaveMap)
	{
		FCombatWave Wave;
		Wave.Enemies = Pair.Value;
		Waves.Add(Wave);

		UE_LOG(LogTemp, Display, TEXT("Wave %d has %d spawns"), Pair.Key, Pair.Value.Num());
	}

	FTimerHandle WaveTimerHandle;

	GetWorld()->GetTimerManager().SetTimer(
		WaveTimerHandle, 
		[this]()
		{
			Server_StartWave(0);
		}, 
		1.5f,    
		false    
	);
}



