// Furkan approves of this


#include "TrapBase.h"

#include "Net/UnrealNetwork.h"

ATrapBase::ATrapBase()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}

void ATrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATrapBase, Active);
}

void ATrapBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATrapBase::DisableTrap_Implementation()
{
	Active = false;
}

