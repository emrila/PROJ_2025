// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTS_DistanceToClosestPlayer.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UBTS_DistanceToClosestPlayer : public UBTService_BlackboardBase
{
	GENERATED_BODY()

	public:
	UBTS_DistanceToClosestPlayer();
	
	UPROPERTY(EditAnywhere)
	float DistanceThreshold = 200.f;
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;


	
};
