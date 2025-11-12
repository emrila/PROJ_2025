#pragma once

#include "CoreMinimal.h"
#include "AttackComponentBase.h"
#include "SlashAttackComp.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API USlashAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	USlashAttackComp();

	virtual void StartAttack() override;

protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack() override;

	UFUNCTION(Server, Reliable)
	virtual void Server_PlayAttackAnim();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlayAttackAnim();

	virtual void CheckForCollisionWithEnemies();

	virtual void Sweep(FVector SweepLocation);

	FTimerHandle SweepTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRadius = 50.f;
	
};
