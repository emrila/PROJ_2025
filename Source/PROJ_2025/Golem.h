// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "Golem.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API AGolem : public AEnemyBase
{
	GENERATED_BODY()
public:
	AGolem();

	UPROPERTY(Replicated, BlueprintReadWrite)
	float RageBuildup = 0;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool Charging;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void HandleHit() override;
	
};
