// Furkan approves of this


#include "ParkourManager.h"

void AParkourManager::OnRoomInitialized(const FRoomInstance& Room)
{
	Super::OnRoomInitialized(Room);
	SpawnLoot();
}
