// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BomberAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API ABomberAIController : public AAIController
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JumpHeight = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 500.0f;
	
	
};
