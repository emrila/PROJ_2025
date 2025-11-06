// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTT_JumpTowardsTarget.generated.h"

/**
 * 
 */
struct FJumpDirection;

UCLASS()

class PROJ_2025_API UBTT_JumpTowardsTarget : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTT_JumpTowardsTarget();


protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	bool TestDirection(const FJumpDirection Direction, const FRotator RotationToPlayer, const APawn* Pawn);
	
	void Jump(float Angle, FRotator RotationToPlayer, APawn* Pawn, float JumpStrength, float ForwardStrength);
	
	void CheckIfLanded();
	
};
