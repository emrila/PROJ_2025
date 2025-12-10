// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Core/UpgradeDisplayData.h"
#include "GameFramework/Actor.h"
#include "StructUtils/InstancedStruct.h"
#include "VotingBooth.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnVotingFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVotingFinishedParams, FInstancedStruct, Data);

USTRUCT(Blueprintable)
struct FCandidate
{
	GENERATED_BODY()

	FORCEINLINE bool operator==(const FCandidate& Other) const
	{
		return (CandidateClass == Other.CandidateClass);
	}
	
	UPROPERTY(BlueprintReadWrite)
	int NumberOfVotes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> CandidateClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName;
	
	UPROPERTY(BlueprintReadWrite)
	FTeamModifierData ModifierData;
};

UCLASS()
class PROJ_2025_API AVotingBooth : public AActor
{
	GENERATED_BODY()
	
public:

	FOnVotingFinished OnVotingFinished;
	FOnVotingFinishedParams OnVotingFinishedParams;
	
	AVotingBooth();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

public:
	
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void StartVote(int NumberOfChoices);

	UPROPERTY(Replicated, BlueprintReadWrite)
	TArray<FCandidate> Candidates;

	UPROPERTY(Replicated, BlueprintReadWrite)
	int Votes = 0;

	UFUNCTION(BlueprintCallable)
	void AttemptVote(FCandidate Candidate);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Vote(FCandidate Candidate);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void CheckResults();
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void AddCandidate(FCandidate Candidate);
	

};
