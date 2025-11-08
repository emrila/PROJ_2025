// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_FindBurrowSpot.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTT_FindBurrowSpot : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTT_FindBurrowSpot();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category="Burrow Settings")
	float SearchRadius = 3000.f;
	
	UPROPERTY(EditAnywhere, Category="Burrow Settings")
	int32 NumSamples = 10;
	
	UPROPERTY(EditAnywhere, Category="Burrow Settings")
	float MinInstantPickDist = 1600.f;
};
