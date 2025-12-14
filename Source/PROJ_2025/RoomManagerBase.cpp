// Furkan approves of this


#include "RoomManagerBase.h"

#include "DroppedItem.h"
#include "ItemBase.h"
#include "LootPicker.h"
#include "RoomLoader.h"
#include "RoomSpawnPoint.h"
#include "WizardGameInstance.h"
#include "RoomExit.h"
#include "WizardGameState.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Controllers/PlayerControllerBase.h"
#include "World/UpgradeSpawner.h"

ARoomManagerBase::ARoomManagerBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

void ARoomManagerBase::OnRoomInitialized(const FRoomInstance& Room)
{
	if (!HasAuthority()) return;

	
	if (AWizardGameState* GameState = Cast<AWizardGameState>(GetWorld()->GetGameState()))
	{
		if (GameState->Health <= 0)
		{
			GameState->RestoreHealth(10.f);
			GameState->OnRep_Health();
		}
		for (APlayerState* Player : GameState->PlayerArray)
		{
			Cast<APlayerCharacterBase>(Player->GetPlayerController()->GetPawn())->ResetIFrame();
		}
	}
	for (TSubclassOf<URoomModifierBase> Mod : Room.ActiveModifierClasses)
	{
		if (!Mod) continue;

		URoomModifierBase* ModInstance = NewObject<URoomModifierBase>(this, Mod);
		ModInstance->RegisterComponent();
		ModInstance->OnRoomEntered(this);
		RoomModifiers.Add(ModInstance);
	}

	UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetGameInstance());
	if (!GI) return;
	TArray<URoomData*> AllRooms;
	if (!GI->RoomLoader->IsDevTest)
	{
		if (GI->RoomLoader->NormalMapPool.Num() <= 4)
		{
			GI->RoomLoader->RefreshPool();
		}
		if (Room.RoomData->RoomType != ERoomType::Parkour)
		{
			AllRooms = GI->RoomLoader->NormalMapPool;
		}else
		{
			AllRooms = GI->RoomLoader->CombatOnly;
		}
	}else
	{
		AllRooms = GI->StaticDevMapPool;
	}
	int BossRoom = GI->RoomLoader->RollForBossRoom();
	bool CampExit = false;
	bool ChoiceRoom = false;
	if (BossRoom == -1)
	{
		ChoiceRoom = GI->RoomLoader->RollForChoiceRoom();
		if (!ChoiceRoom)
		{
			CampExit = GI->RoomLoader->RollForCampRoom();
		}else
		{
			GI->RoomLoader->RollForCampRoom(true);
		}
	}

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
	if (FMath::FRand() <= 0.75f && RoomExits.Num() > 1 || BossRoom != -1)
	{
		int32 IndexToDelete = FMath::RandRange(0, RoomExits.Num() - 1);
		RoomExits[IndexToDelete]->Destroy();
		RoomExits.RemoveAt(IndexToDelete);
	}
	if (!CampExit  && !ChoiceRoom && FMath::FRand() <= 0.1f && RoomExits.Num() > 1 || BossRoom != -1)
	{
		int32 IndexToDelete = FMath::RandRange(0, RoomExits.Num() - 1);
		RoomExits[IndexToDelete]->Destroy();
		RoomExits.RemoveAt(IndexToDelete);
	}
	
	TArray<URoomData*> ChosenRooms;

	if (CampExit)
	{
		ChosenRooms.Add(GI->CampRoom);
	}
	if (ChoiceRoom)
	{
		ChosenRooms.Add(GI->ChoiceRoom);
	}
	if (BossRoom != -1)
	{
		ChosenRooms.Add(GI->BossRooms[BossRoom]);
	}

	if (Room.RoomData && AllRooms.Contains(Room.RoomData))
	{
		AllRooms.Remove(Room.RoomData);
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
	
	UE_LOG(LogTemp, Display, TEXT("Rooms exits %d"), RoomExits.Num());
	while (ChosenRooms.Num() < RoomExits.Num())
	{
		UE_LOG(LogTemp, Display, TEXT("Currently chosen: %d"), ChosenRooms.Num());
		if (AllRooms.Num() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Ran out of rooms before filling exits!"));
			break; 
		}
		int32 RandIndex = FMath::RandRange(0, AllRooms.Num() - 1);

		URoomData* RandomRoom = AllRooms[RandIndex];
		
		ChosenRooms.Add(RandomRoom);
		
		AllRooms.RemoveAt(RandIndex);
	}

	for (int32 i = 0; i < RoomExits.Num(); ++i)
	{
		if (!RoomExits[i]) continue;

		URoomData* RoomData = ChosenRooms.IsValidIndex(i) ? ChosenRooms[i] : nullptr;
		FRoomInstance RoomInstance;
		RoomInstance.RoomData = RoomData;
		if (!RoomData) continue;
		const float ChanceForModifier = GI->RoomLoader->ClearedRooms * GI->RoomLoader->ChanceForModifiers;
		if (FMath::FRand() <= ChanceForModifier)
		{
			if (FRoomModifierArray* FoundMods = GI->AvailableModsForRoomType.Find(RoomData->RoomType))
			{
				TArray<TSubclassOf<URoomModifierBase>>& Mods = FoundMods->Modifiers;

				if (Mods.Num() > 0)
				{
					int32 RandomIndex = FMath::RandRange(0, Mods.Num() - 1);
					TSubclassOf<URoomModifierBase> ModClass = Mods[RandomIndex];
					
					RoomInstance.ActiveModifierClasses.Add(ModClass);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("No modifiers found for RoomType %d"), (uint8)RoomData->RoomType);
			}
		}
		RoomExits[i]->LinkedRoomInstance = RoomInstance;

		if (RoomData)
		{
			RoomExits[i]->OnRoomLinked();
		}
	}
	AActor* SpawnPoint = UGameplayStatics::GetActorOfClass(GetWorld(), ARoomSpawnPoint::StaticClass());

	if (!SpawnPoint) return;

	ARoomSpawnPoint* Spawn = Cast<ARoomSpawnPoint>(SpawnPoint);
	if (!Spawn) return;
	
	const FTransform SpawnTransform = Spawn->ArrowComponent->GetComponentTransform();
	FRotator SpawnRot = Spawn->ArrowComponent->GetComponentRotation();
	
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
		Cast<APlayerControllerBase>(PC)->Client_SetSpawnRotation(SpawnRot);
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
	UE_LOG(LogTemp, Warning, TEXT("SPAWNING LOOT!"));
	if (LootSpawnLocation)
	{
		LootSpawnLocation->TriggerSpawn();
		LootSpawnLocation->OnCompletedAllUpgrades.AddDynamic(this, &ARoomManagerBase::EnableExits);
		for (URoomModifierBase* Mod : RoomModifiers)
		{
			Mod->OnLootSpawned();
		}
		if (RoomModifiers.Num() > 0)
		{
			UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetGameInstance());
			ELootTier Tier;
			FItemDataRow* RandomLoot = FLootPicker::PickLoot(Tier);
			UItemBase* ItemInstance = NewObject<UItemBase>(this,RandomLoot->ItemClass);
			ItemInstance->LootTier = Tier;
			ItemInstance->DroppedMesh = RandomLoot->DroppedMesh;
			ItemInstance->Icon = RandomLoot->Icon;
			ItemInstance->Initialize();
			ADroppedItem* DroppedItem = GetWorld()->SpawnActor<ADroppedItem>(GI->RoomLoader->DroppedItemClass, LootSpawnLocation->GetActorLocation() + FVector(0.f,0.f,125.f), LootSpawnLocation->GetActorRotation());
			DroppedItem->ItemMesh->SetStaticMesh(RandomLoot->DroppedMesh);
			DroppedItem->ItemData = ItemInstance;
			DroppedItem->Initialize();
		}
	}else
	{
		EnableExits();
	}
	if (AWizardGameState* GameState = Cast<AWizardGameState>(GetWorld()->GetGameState()))
	{
		if (GameState->Health <= 0)
		{
			GameState->RestoreHealth(10.f);
			GameState->OnRep_Health();
		}
		
		if (LootSpawnLocation)
		{
			LootSpawnLocation->SetTotalUpgradeNeededForCompletion(GameState->CurrentPlayerCount);
		}
	}
}

void ARoomManagerBase::EnableExits()
{
	TArray<AActor*> FoundExits;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARoomExit::StaticClass(), FoundExits);

	if (LootSpawnLocation)
	{
		LootSpawnLocation->OnCompletedAllUpgrades.RemoveDynamic(this, &ARoomManagerBase::EnableExits);
	}

	for (URoomModifierBase* Mod : RoomModifiers)
	{
		Mod->OnExitsUnlocked();
	}
	
	for (AActor* Actor : FoundExits)
	{
		if (ARoomExit* Exit = Cast<ARoomExit>(Actor))
		{
			Exit->CanExit = true;
			Exit->EnableExit();
		}
	}
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
