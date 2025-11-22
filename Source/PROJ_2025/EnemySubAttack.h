// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySubAttack.generated.h"

UCLASS()
class PROJ_2025_API AEnemySubAttack : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(BlueprintReadWrite)
	float DamageMultiplier = 1.f;
	

};
