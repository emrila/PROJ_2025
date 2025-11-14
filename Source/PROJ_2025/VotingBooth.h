// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VotingBooth.generated.h"



struct FCandidate
{
	int NumberOfVotes;
	UObject* Object;
};

UCLASS()
class PROJ_2025_API AVotingBooth : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVotingBooth();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, NetMulticast, Reliable)
	void StartVote(int NumberOfChoices);

	

	

};
