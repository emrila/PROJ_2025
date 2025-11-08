// Furkan approves of this


#include "BTT_SetIsBurrowingFalse.h"

#include "AIController.h"
#include "CactusCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_SetIsBurrowingFalse::UBTT_SetIsBurrowingFalse()
{
	NodeName = "Set IsBurrowing False";
}

EBTNodeResult::Type UBTT_SetIsBurrowingFalse::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	
	if (ACactusCharacter* Cactus = Cast<ACactusCharacter>(AIPawn))
	{
		Cactus->bIsBurrowing = false;
		Blackboard->SetValueAsBool("IsBurrowing", false);
		
	}
	
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
