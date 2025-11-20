// Furkan approves of this


#include "VotingBooth.h"

#include "EditorMetadataOverrides.h"
#include "NiagaraValidationRule.h"
#include "GameFramework/GameStateBase.h"
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

void AVotingBooth::AttemptVote(FCandidate Candidate)
{
	Vote(Candidate);
}



void AVotingBooth::AddCandidate_Implementation(FCandidate Candidate)
{
	Candidates.Add(Candidate);
}


void AVotingBooth::Vote_Implementation(FCandidate Candidate)
{

	if (!HasAuthority())
	{
		return;
	}
	if (Candidates.Contains(Candidate))
	{
		Candidate.NumberOfVotes += 1;
		Votes += 1;

		UE_LOG(LogTemp, Warning, TEXT("Current votes: %d"), Votes);
	}
	CheckResults();
	
}
void AVotingBooth::CheckResults_Implementation()
 {
	if (Votes == GetWorld()->GetGameState()->PlayerArray.Num())
	{
		for (FCandidate CurrentCandidate : Candidates)
		{
			if (CurrentCandidate.NumberOfVotes >= 2)
			{
				const FActorSpawnParameters SpawnInfo;
				GetWorld()->SpawnActor<AActor>(
					CurrentCandidate.CandidateClass,
					GetActorLocation() + FVector(0,0,200),
					GetActorRotation(),
					SpawnInfo
				);
				this->Destroy();
				return;
			}
		}
		
		int32 Index = FMath::RandRange(0, Candidates.Num() - 1);

		FActorSpawnParameters SpawnInfo;
		
		GetWorld()->SpawnActor<AActor>(
			Candidates[Index].CandidateClass,
			GetActorLocation() + FVector(0,0,200),
			GetActorRotation(),
			SpawnInfo
		);

		this->Destroy();
	}
 }

void AVotingBooth::StartVote_Implementation(int NumberOfChoices)
{
	
}

