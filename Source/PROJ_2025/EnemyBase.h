// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "CombatManager.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

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

	UPROPERTY()
	ACombatManager* CombatManager;

	UFUNCTION(BlueprintImplementableEvent)
	void HitFeedback();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Stats")
	float Health;

	virtual void HandleDeath();

	UFUNCTION(NetMulticast, Reliable)
	virtual void HandleHit(struct FDamageEvent const& DamageEvent, AActor* DamageCauser);
	

};
