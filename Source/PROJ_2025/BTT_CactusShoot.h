// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_CactusShoot.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_CactusShoot : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	public:
	UBTT_CactusShoot();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
