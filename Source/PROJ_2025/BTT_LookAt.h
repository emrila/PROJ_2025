// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_LookAt.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_LookAt : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
    UBTT_LookAt();

protected:

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
