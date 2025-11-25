// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MushroomAIController.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FJumpDirection
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float Pair")
	float Angle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Float Pair")
	float ChanceToStay;

};

UCLASS()
class PROJ_2025_API AMushroomAIController : public AAIController
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FJumpDirection> JumpDirections;

	UFUNCTION(BlueprintCallable)
	void StartAttackCooldown();
	

	bool AttackIsOnCooldown = false;

private:
	void ResetAttackCooldown();
	FTimerHandle AttackCooldownHandle;

	
};
