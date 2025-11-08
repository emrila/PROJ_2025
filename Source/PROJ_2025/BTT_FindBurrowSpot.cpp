// Furkan approves of this


#include "BTT_FindBurrowSpot.h"

#include "AIController.h"
#include "CactusCharacter.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_FindBurrowSpot::UBTT_FindBurrowSpot()
{
	NodeName = "Find Burrow Spot";
}

EBTNodeResult::Type UBTT_FindBurrowSpot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
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
	if (PlayerPawns.Num() == 0) return EBTNodeResult::Failed;
	
	const FVector Origin = AIPawn->GetActorLocation();
	UWorld* World = AIPawn->GetWorld();

	FVector BestPoint = Origin;
	float BestDist = 0.f;
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	
	for (int32 i = 0; i < NumSamples; i++)
	{
		FNavLocation RandomLoc;
		if (!NavSys->GetRandomReachablePointInRadius(Origin, SearchRadius, RandomLoc))
			continue;

		const FVector Candidate = RandomLoc.Location;
		bool bHasLOS = false;

		for (AActor* Player : PlayerPawns)
		{
			if (AICon->LineOfSightTo(Player, Candidate, true))
			{
				bHasLOS = true;
				break;
			}
		}
		if (!bHasLOS)
		{
			continue; 
		}

		float ClosestDist = TNumericLimits<float>::Max();
		for (AActor* Player : PlayerPawns)
		{
			const float Dist = FVector::Dist(Candidate, Player->GetActorLocation());
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
			}
		}

		//DrawDebugSphere(World, Candidate, 50, 12, FColor::Blue, false, 3.f);
		//DrawDebugString(World, Candidate + FVector(0,0,50), FString::Printf(TEXT("%.0f"), ClosestDist), nullptr, FColor::White, 3.f);

		if (ClosestDist >= MinInstantPickDist)
		{
			BestPoint = Candidate;
			break;
		}
		if (ClosestDist > BestDist)
		{
			BestPoint = Candidate;
			BestDist = ClosestDist;
		}
	}
	if (BestPoint == Origin)
	{
		return EBTNodeResult::Failed;
	}
	if (ACactusCharacter* Cactus = Cast<ACactusCharacter>(AIPawn))
	{
		Cactus->bIsBurrowing = true;
		if (AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor")))
			{
			FVector Direction = (TargetActor->GetActorLocation() - Cactus->GetActorLocation()).GetSafeNormal();
			FRotator NewRotation = Direction.Rotation();

			Cactus->SetActorRotation(NewRotation);
		}
	}
	Blackboard->SetValueAsBool("IsBurrowing", true);
	Blackboard->SetValueAsVector("BurrowLocation", BestPoint);

	return EBTNodeResult::Succeeded;
}
