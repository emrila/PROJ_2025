// Furkan approves of this


#include "RoomManagerBase.h"

#include "LootSpawnLocation.h"
#include "RoomLoader.h"
#include "RoomSpawnPoint.h"
#include "WizardGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "World/UpgradeSpawner.h"

ARoomManagerBase::ARoomManagerBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ARoomManagerBase::OnRoomInitialized()
{
	if (!HasAuthority()) return;

	UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetGameInstance());
	if (!GI) return;

	TArray<URoomData*> AllRooms = GI->GetAllRoomData();

	UE_LOG(LogTemp, Warning, TEXT("Found %d rooms"), AllRooms.Num());

	TArray<AActor*> FoundExits;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomExit::StaticClass(), FoundExits);

	TArray<ARoomExit*> RoomExits;
	for (AActor* Actor : FoundExits)
	{
		if (ARoomExit* Exit = Cast<ARoomExit>(Actor))
		{
			RoomExits.Add(Exit);
		}
	}
	TArray<URoomData*> ChosenRooms;

	if (GI->RoomLoader->CurrentRoom != nullptr)
	{
		AllRooms.Remove(GI->RoomLoader->CurrentRoom);
	}
	if (AllRooms.Num() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("No rooms found..."));
		return;
	}
	for (int32 i = 0; i < AllRooms.Num(); i++)
	{
		URoomData* RandomRoom = AllRooms[i];
		UE_LOG(LogTemp, Display, TEXT("ROOM: %s"), *RandomRoom->GetName());
	}
	
	
	for (int32 i = 0; i < RoomExits.Num(); i++)
	{
		URoomData* RandomRoom = AllRooms[FMath::RandRange(0, AllRooms.Num() - 1)];
		AllRooms.Remove(RandomRoom);
		ChosenRooms.Add(RandomRoom);
	}

	for (int32 i = 0; i < RoomExits.Num(); ++i)
	{
		if (!RoomExits[i]) continue;

		URoomData* RoomData = ChosenRooms.IsValidIndex(i) ? ChosenRooms[i] : nullptr;
		RoomExits[i]->LinkedRoomData = RoomData;

		if (RoomData)
		{
			RoomExits[i]->OnRoomLinked();
		}
	}
	AActor* SpawnPoint = UGameplayStatics::GetActorOfClass(GetWorld(), ARoomSpawnPoint::StaticClass());

	if (!SpawnPoint) return;

	const FTransform SpawnTransform = SpawnPoint->GetActorTransform();
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		APawn* PlayerPawn = PC->GetPawn();
		if (!PlayerPawn) continue;
		
		FVector Offset = FVector(0.f, It.GetIndex() * 100.f, 0.f);
		FTransform AdjustedTransform = SpawnTransform;
		AdjustedTransform.AddToTranslation(Offset);

		PlayerPawn->SetActorTransform(AdjustedTransform);
		UE_LOG(LogTemp, Display, TEXT("Teleported %s to room spawn point."), *PlayerPawn->GetName());
	}

	AActor* LootSpawnLoc = UGameplayStatics::GetActorOfClass(GetWorld(), AUpgradeSpawner::StaticClass());
	if (LootSpawnLoc)
	{
		LootSpawnLocation = Cast<AUpgradeSpawner>(LootSpawnLoc);
	}
}

void ARoomManagerBase::SpawnLoot()
{
	LootSpawnLocation->TriggerSpawn();
}


/*
public class dontRunThis
	{
		String name = "Emma";
		Loot loot = Loot(String TypeOfLoot, float modifierAmount);
		SpawnLoot(Loot);
		//hmmmmmm??????????
		if (LootSpawnLocation == null)
			do
			{
				SpawnLoot();
			}
			while (LootSpawnLocation == nullptr);
		else if (LootSpawnLocation == nullptr && Loot != nullptr)
		{
			return;
		}
	}
*/