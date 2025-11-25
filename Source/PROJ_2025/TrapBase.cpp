// Furkan approves of this


#include "TrapBase.h"

#include "Net/UnrealNetwork.h"

// Sets default values
ATrapBase::ATrapBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ATrapBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)
{
	DOREPLIFETIME(ATrapBase, Active);
}

// Called when the game starts or when spawned
void ATrapBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATrapBase::DisableTrap_Implementation()
{
	Active = true;
}

