// Furkan approves of this


#include "ChoiceRoomManager.h"

#include "VotingBooth.h"
#include "Kismet/GameplayStatics.h"

void AChoiceRoomManager::OnRoomInitialized(const FRoomInstance& Room)
{
	Super::OnRoomInitialized(Room);

	if (AActor* VoteActor = UGameplayStatics::GetActorOfClass(GetWorld(), AVotingBooth::StaticClass()))
	{
		if (AVotingBooth* VotingBooth = Cast<AVotingBooth>(VoteActor))
		{
			Booth = VotingBooth;
			Booth->OnVotingFinished.AddDynamic(this, &AChoiceRoomManager::EnableExits);
			
		}
	}
}

void AChoiceRoomManager::EnableExits()
{
	Super::EnableExits();
	Booth->Destroy();
}
