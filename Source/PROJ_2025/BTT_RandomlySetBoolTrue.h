// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_RandomlySetBoolTrue.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_RandomlySetBoolTrue : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	UBTT_RandomlySetBoolTrue();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UPROPERTY(EditAnywhere)
	float ChanceToSetTrue = 0.1f;
};