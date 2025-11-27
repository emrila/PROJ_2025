#pragma once

#include "CoreMinimal.h"
#include "../AttackComponentBase.h"
#include "SlashAttackComp.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API USlashAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	USlashAttackComp();

	virtual void StartAttack() override;
	
	virtual void StartAttack(const float NewDamageAmount, float NewAttackCooldown) override;

protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack() override;

	UFUNCTION(Server, Reliable)
	virtual void Server_PlayAttackAnim();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlayAttackAnim();

	virtual void CheckForCollisionWithEnemies();

	UFUNCTION(Server, Reliable)
	virtual void Sweep(FVector SweepLocation);

	virtual float GetAttackCooldown() const override;

	virtual float GetDamageAmount() const override;

	FTimerHandle SweepTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackMontage;
	
	float AttackRadius = 100.f;
};
