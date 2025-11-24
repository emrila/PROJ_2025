// Furkan approves of this


#include "ParkourTimerMod.h"

#include "ParkourManager.h"

void UParkourTimerMod::OnRoomEntered(ARoomManagerBase* InRoomManager)
{
	Super::OnRoomEntered(InRoomManager);

	if (AParkourManager* ParkourManager = Cast<AParkourManager>(RoomManager))
	{
		Timer = ParkourManager->TimerIfTimeTrial;
	}
}
