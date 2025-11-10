// Furkan approves of this


#include "RoomManagerBase.h"

#include "WizardGameInstance.h"
#include "Kismet/GameplayStatics.h"

ARoomManagerBase::ARoomManagerBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ARoomManagerBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetGameInstance());
	if (!GI) return;

	TArray<URoomData*> AllRooms = GI->GetAllRoomData();

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

	for (int32 i = 0; i < AllRooms.Num(); i++)
	{
		URoomData* RandomRoom = AllRooms[i];
		UE_LOG(LogTemp, Display, TEXT("ROOM: %s"), *RandomRoom->GetName());
	}
	if (GI->CurrentRoom != nullptr)
	{
		AllRooms.Remove(GI->CurrentRoom);
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
	
}


