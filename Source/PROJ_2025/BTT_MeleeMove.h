// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTT_MeleeMove.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_MeleeMove : public UBTTask_MoveTo
{
	GENERATED_BODY()

	UBTT_MeleeMove();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	FTimerHandle TimerHandle;

	FTimerHandle TimerHandleTimeout;

	UPROPERTY()
	APawn* Mushroom;

	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	UBehaviorTreeComponent* OwnerComponent;
	
	UPROPERTY()
	UBlackboardComponent* Blackboard;
	
	void CheckIfOutOfRangeFromTarget();
	

	void TimedOut();

	UPROPERTY(EditAnywhere, Category = "AttackRange")
	float InterruptRange = 1000.f;

	UPROPERTY(EditAnywhere, Category = "AttackRange")
	float TimeOutTimer = 4.f;
};
