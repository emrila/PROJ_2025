// Furkan approves of this


#include "Golem.h"

#include "Net/UnrealNetwork.h"

AGolem::AGolem()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AGolem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGolem, RageBuildup);
}

void AGolem::HandleHit()
{
	Super::HandleHit();
	
}


