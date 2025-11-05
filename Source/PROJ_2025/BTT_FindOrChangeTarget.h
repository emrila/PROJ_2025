// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_FindOrChangeTarget.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_FindOrChangeTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTT_FindOrChangeTarget();

private:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "Target Selection", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SwitchChance = 0.1f;
};
