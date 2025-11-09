// Furkan approves of this


#include "RoomManagerBase.h"

#include "WizardGameInstance.h"

ARoomManagerBase::ARoomManagerBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ARoomManagerBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	// Example: Get your custom GameInstance for persistence
	UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetGameInstance());
	if (!GI) return;

	// Example pool of all possible room data assets (could come from GI or DataTable)
	TArray<URoomData*> AllRooms = GI->GetAllRoomData();

	// Shuffle or filter rooms based on logic
	TArray<URoomData*> ChosenRooms;
	const int32 NumChoices = FMath::Min(RoomExits.Num(), AllRooms.Num());

	for (int32 i = 0; i < NumChoices; ++i)
	{
		URoomData* RandomRoom = AllRooms[FMath::RandRange(0, AllRooms.Num() - 1)];
		ChosenRooms.Add(RandomRoom);
	}

	// Assign each exit one of the chosen rooms
	for (int32 i = 0; i < RoomExits.Num(); ++i)
	{
		if (!RoomExits[i]) continue;

		URoomData* RoomData = ChosenRooms.IsValidIndex(i) ? ChosenRooms[i] : nullptr;
		RoomExits[i]->LinkedRoomData = RoomData;

		if (RoomData)
		{
			// Call blueprint event to update visuals/icons
			RoomExits[i]->OnRoomLinked();
		}
	}
	
}


