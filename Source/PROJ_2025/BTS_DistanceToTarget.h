// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTS_DistanceToTarget.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTS_DistanceToTarget : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTS_DistanceToTarget();

protected:

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "AttackRange")
	float AttackRange = 500.f; 
};
	

