// Furkan approves of this


#include "BTT_RandomlySetBoolTrue.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTT_RandomlySetBoolTrue::UBTT_RandomlySetBoolTrue()
{
	NodeName = "Randomly Set Bool True";
}

EBTNodeResult::Type UBTT_RandomlySetBoolTrue::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}
	
	const float Roll = FMath::FRand(); 
	if (Roll <= ChanceToSetTrue)
	{
		Blackboard->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}

	return EBTNodeResult::Succeeded;
}
