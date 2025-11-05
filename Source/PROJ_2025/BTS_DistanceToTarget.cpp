// Furkan approves of this


// BTS_DistanceToTarget.cpp

#include "BTS_DistanceToTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "MushroomAIController.h"
#include "GameFramework/Pawn.h"

UBTS_DistanceToTarget::UBTS_DistanceToTarget()
{
	NodeName = "Check Distance to Target";
}

void UBTS_DistanceToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon) return;

	AMushroomAIController* MushroomAIController = Cast<AMushroomAIController>(AICon);

	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn) return;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard) return;

	bool AttackIsOnCooldown = Blackboard->GetValueAsBool("AttackIsOnCooldown");
	
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(GetSelectedBlackboardKey()));
	if (!TargetActor)
	{
		Blackboard->SetValueAsBool("canAttack", false);
		return;
	}
	
	float Distance = FVector::Dist(AIPawn->GetActorLocation(), TargetActor->GetActorLocation());
	Blackboard->SetValueAsBool("canAttack", Distance <= AttackRange && !AttackIsOnCooldown);
	
}
