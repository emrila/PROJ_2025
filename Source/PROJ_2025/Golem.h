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

	UPROPERTY(Replicated, BlueprintReadOnly)
	float RageBuildup = 0;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool Charging;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool AttackIsOnCooldown = false;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsAttacking;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsRunning = false;

	virtual void HandleHit() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	
	
};
