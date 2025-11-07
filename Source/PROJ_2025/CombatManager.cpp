// Furkan approves of this


#include "CombatManager.h"

#include "BomberCharacter.h"
#include "EnemySpawn.h"
#include "MushroomCharacter.h"
#include "Net/UnrealNetwork.h"

void ACombatManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ACombatManager, RemainingEnemies);
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
	if (!Waves.IsValidIndex(index)) return;
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
			if (AMushroomCharacter* Mush = Cast<AMushroomCharacter>(SpawnedActor))
			{
				Mush->CombatManager = this;
			}
			if (ABomberCharacter* Bomber = Cast<ABomberCharacter>(SpawnedActor))
			{
				Bomber->CombatManager = this;
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
		UE_LOG(LogTemp, Warning, TEXT("wave2"));
		Server_StartWave(1);
	}
}

// Called every frame
void ACombatManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

