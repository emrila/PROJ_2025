// Furkan approves of this


#include "CombatManager.h"

#include "BomberCharacter.h"
#include "EnemySpawn.h"
#include "MushroomCharacter.h"
#include "RoomLoader.h"
#include "WizardGameInstance.h"
#include "WizardGameState.h"
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
	UE_LOG(LogTemp, Display, TEXT("StartWave"));
	if (!Waves.IsValidIndex(index))
	{
		SpawnLoot();
		return;
	};
	UE_LOG(LogTemp, Display, TEXT("Wave: %d"), index);

	int Sum = 0;
	
	TMap<EEnemyType, TArray<AEnemySpawn*>> EnemyLocationsCopy = EnemyLocations;
	
	UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetGameInstance());
	float DungeonScaling = GI->RoomLoader->GetDungeonScaling();
	
	for (const TPair<EEnemyType, int> Pair : Waves[index].EnemyCounts)
	{
		TArray<AEnemySpawn*> Spawns = EnemyLocationsCopy[Pair.Key];
		
		const int MaxSpawns = FMath::RoundToInt(Pair.Value * MaxSpawnMultiplier);
		UE_LOG(LogTemp, Display, TEXT("spawnmult: %f"), MaxSpawnMultiplier);
		for (int i = 0; i < MaxSpawns; i++)
		{
			if (Spawns.Num() == 0)
			{
				UE_LOG(LogTemp, Error, TEXT("RAN OUT OF %s TO SPAWN"),*UEnum::GetValueAsString(Pair.Key));
				break;
			}
			const int32 RandomIndex = FMath::RandRange(0, Spawns.Num() - 1);
			const AEnemySpawn* SpawnPoint = Spawns[RandomIndex];
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
				Enemy->Health = Enemy->Health * DungeonScaling;
				Enemy->DamageMultiplier = DungeonScaling;
				UE_LOG(LogTemp, Warning, TEXT("Spawned enemy with scaling %f"), DungeonScaling);
				Sum++;
				
			}
			Spawns.RemoveAt(RandomIndex);

		}
	}
	RemainingEnemies = Sum;
}
void ACombatManager::RegisterEnemyDeath_Implementation()
{
	if (!HasAuthority()) return;

	if (AWizardGameState* GameState = Cast<AWizardGameState>(GetWorld()->GetGameState()))
	{
		if (GameState->Health > 0)
		{
			GameState->RestoreHealth(GameState->LifeStealMultiplier - 1.f);
		}
	}
	
	RemainingEnemies--;
	if (RemainingEnemies == 0)
	{
		WaveIndex++;
		Server_StartWave(WaveIndex);
	}
}

void ACombatManager::OnRoomInitialized(const FRoomInstance& Room)
{
	Super::OnRoomInitialized(Room);
	TArray<AActor*> FoundSpawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemySpawn::StaticClass(), FoundSpawns);

	EnemyLocations.Empty();
	WaveIndex = 0;
	for (AActor* Actor : FoundSpawns)
	{
		if (AEnemySpawn* Spawn = Cast<AEnemySpawn>(Actor))
		{
			EnemyLocations.FindOrAdd(Spawn->EnemyType).Add(Spawn);
		}
	}

	FTimerHandle WaveTimerHandle;

	GetWorld()->GetTimerManager().SetTimer(
		WaveTimerHandle, 
		[this]()
		{
			Server_StartWave(0);
		}, 
		3.5f,    
		false    
	);
}



