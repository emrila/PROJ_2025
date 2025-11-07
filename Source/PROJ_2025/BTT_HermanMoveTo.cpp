// Furkan approves of this


#include "BTT_HermanMoveTo.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

EBTNodeResult::Type UBTT_HermanMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	OwnerComponent = &OwnerComp;
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;

	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn)
		return EBTNodeResult::Failed;

	Mushroom = AIPawn;

	auto CM = Cast<ACharacter>(AIPawn)->GetCharacterMovement();

	CM->MaxWalkSpeed = 400.f;

	Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
		return EBTNodeResult::Failed;

	TargetActor = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor"));
	
	Super::ExecuteTask(OwnerComp, NodeMemory);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBTT_HermanMoveTo::CheckIfSameHeightAsTarget, 0.1f, true);

	
	return EBTNodeResult::InProgress;
}

void UBTT_HermanMoveTo::CheckIfSameHeightAsTarget()
{
	if (!Mushroom || !TargetActor)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	if (TargetActor->GetActorLocation().Z <= Mushroom->GetActorLocation().Z + 50.f && TargetActor->GetActorLocation().Z >= Mushroom->GetActorLocation().Z - 100.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		Blackboard->SetValueAsBool("IsOnDifferentZ", false);
		auto CM = Cast<ACharacter>(Mushroom)->GetCharacterMovement();
		CM->MaxWalkSpeed = 600.f;
		FinishLatentTask(*OwnerComponent, EBTNodeResult::Succeeded);
	}
}
