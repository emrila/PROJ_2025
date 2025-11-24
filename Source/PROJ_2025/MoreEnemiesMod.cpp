// Furkan approves of this


#include "MoreEnemiesMod.h"

#include "CombatManager.h"

void UMoreEnemiesMod::OnRoomEntered(ARoomManagerBase* InRoomManager)
{
	Super::OnRoomEntered(InRoomManager);
	UE_LOG(LogTemp, Display, TEXT("OnRoomEntered"));
	if (ACombatManager* CombatManager = Cast<ACombatManager>(RoomManager))
	{
		CombatManager->MaxSpawnMultiplier = 1.f + 0.2f*ModLevel;
		UE_LOG(LogTemp, Error, TEXT("SetTo1.2"));
	}
}
