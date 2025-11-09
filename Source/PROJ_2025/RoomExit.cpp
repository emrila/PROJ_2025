// Furkan approves of this


#include "RoomExit.h"

#include "Net/UnrealNetwork.h"

// Sets default values
ARoomExit::ARoomExit()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ARoomExit::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARoomExit, LinkedRoomData);
}

// Called when the game starts or when spawned
void ARoomExit::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARoomExit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

