// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "CombatManager.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"


UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateHealthbar, float, CurrentHealth);

UCLASS()

class PROJ_2025_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Replicated)
	FUpdateHealthbar UpdateHealthbar;

	UPROPERTY()
	ACombatManager* CombatManager;

	UFUNCTION(BlueprintImplementableEvent)
	void HitFeedback();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, ReplicatedUsing=OnRep_Health, Category="Stats")
	float Health;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY(Replicated)
	bool HasDied;
	
	UPROPERTY(BlueprintReadOnly)
	float DamageMultiplier = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsDummy = false;

	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UNiagaraSystem* DeathEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animations")
	UAnimMontage* DeathMontage;

	UFUNCTION()
	void FinishDeath();
	
	virtual void HandleDeath();

	UFUNCTION(NetMulticast, Reliable)
	virtual void HandleHit(struct FDamageEvent const& DamageEvent, AActor* DamageCauser);

	UFUNCTION(NetMulticast, Reliable)
	virtual void SpawnDeathEffect();
};
