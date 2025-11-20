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

	UPROPERTY(ReplicatedUsing= OnRep_Death, BlueprintReadWrite)
	bool Dead;

	UFUNCTION(BlueprintImplementableEvent)
	void OnRep_Death();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void HandleHit(struct FDamageEvent const& DamageEvent, AActor* DamageCauser) override;

	virtual void HandleDeath() override;
	
	void Die();
	
};
