// Furkan approves of this


#include "BTS_DistanceToClosestPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTS_DistanceToClosestPlayer::UBTS_DistanceToClosestPlayer()
{
	NodeName = "Distance To Closest Player";
}

void UBTS_DistanceToClosestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		UE_LOG(LogTemp, Error, TEXT("AIController not found"));
		return;
	}

	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Pawn not found"));
		return;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		UE_LOG(LogTemp, Error, TEXT("Blackboard not found"));
		return;
	}

	TArray<AActor*> PlayerPawns;
	for (FConstPlayerControllerIterator It = AIPawn->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() && PC->GetPawn() != AIPawn)
		{
			PlayerPawns.Add(PC->GetPawn());
		}
	}
	if (PlayerPawns.Num() == 0) return;
	
	const FVector AI_Location = AIPawn->GetActorLocation();

	for (AActor* Player : PlayerPawns)
	{
		if (FVector::Dist(AI_Location, Player->GetActorLocation()) < DistanceThreshold)
		{
			Blackboard->SetValueAsBool("SpikeExplosion", true);
			return;
		}
	}
	Blackboard->SetValueAsBool("SpikeExplosion", false);
}
