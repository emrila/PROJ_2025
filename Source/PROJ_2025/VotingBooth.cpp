// Furkan approves of this


#include "VotingBooth.h"

#include "NiagaraValidationRule.h"
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
	DOREPLIFETIME(AVotingBooth, Votes);

	
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

void AVotingBooth::AddCandidate_Implementation(FCandidate Candidate)
{
	Candidates.Add(Candidate);
}


void AVotingBooth::Vote_Implementation(FCandidate Candidate)
{
	if (Candidates.Contains(Candidate))
	{
		Candidate.NumberOfVotes += 1;
		Votes += 1;

		UE_LOG(LogTemp, Warning, TEXT("Current votes: %d"), Votes);
	}

}

void AVotingBooth::StartVote_Implementation(int NumberOfChoices)
{
	
}

