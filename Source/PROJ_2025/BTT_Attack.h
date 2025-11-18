// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_Attack.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_Attack : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTT_Attack();

	UPROPERTY(EditAnywhere, Category = "OverallAttackStrength")
	float AttackStrength = 1.f;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
