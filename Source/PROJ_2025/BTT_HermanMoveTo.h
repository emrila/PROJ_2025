// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTT_HermanMoveTo.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_HermanMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	FTimerHandle TimerHandle;

	UPROPERTY()
	APawn* Mushroom;

	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	UBehaviorTreeComponent* OwnerComponent;
	
	UPROPERTY()
	UBlackboardComponent* Blackboard;
	
	void CheckIfSameHeightAsTarget();
};
