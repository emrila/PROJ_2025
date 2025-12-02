#include "RoomModifierBase.h"

#include "WizardGameState.h"
#include "Player/Controllers/PlayerControllerBase.h"

void URoomModifierBase::OnRoomEntered(ARoomManagerBase* InRoomManager)
{
	if (!GetOwner()->HasAuthority()) return;
	
	RoomManager = InRoomManager;
}

void URoomModifierBase::OnExitsUnlocked()
{
}

void URoomModifierBase::RegisterClient_Implementation()
{
	ClientsReady++;
	UE_LOG(LogTemp, Warning, TEXT("Host Counted clients ready: %d"), ClientsReady);
	if (AWizardGameState* GS = Cast<AWizardGameState>(GetWorld()->GetGameState()))
	{
		if (GS->PlayerArray.Num() == ClientsReady)
		{
			
			OnAllClientsReady();
		}
	}
}

void URoomModifierBase::OnAllClientsReady()
{
	UE_LOG(LogTemp, Warning, TEXT("ALL CLIENTS READY BASE"));
}
