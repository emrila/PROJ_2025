// Furkan approves of this


#include "CombatManager.h"

#include "EnemySpawn.h"

// Sets default values
ACombatManager::ACombatManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ACombatManager::Server_StartWave_Implementation()
{
	StartWave_Internal();
}


void ACombatManager::StartWave_Internal()
{
	if (!Waves.IsValidIndex(0)) return;

	for (AEnemySpawn* SpawnPoint : Waves[0].Enemies)
	{
		if (SpawnPoint && SpawnPoint->EnemyClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			GetWorld()->SpawnActor<AActor>(
				SpawnPoint->EnemyClass,
				SpawnPoint->GetActorLocation(),
				SpawnPoint->GetActorRotation()
			);
		}
	}
}

// Called every frame
void ACombatManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

