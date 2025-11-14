// Furkan approves of this


#include "VotingBooth.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AVotingBooth::AVotingBooth()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AVotingBooth::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	
}

// Called when the game starts or when spawned
void AVotingBooth::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVotingBooth::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AVotingBooth::StartVote_Implementation(int NumberOfChoices)
{
	
}

