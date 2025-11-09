// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_CactusSpikeExplosion.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_CactusSpikeExplosion : public UBTTaskNode
{
	GENERATED_BODY()
	public:
	UBTT_CactusSpikeExplosion();
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
