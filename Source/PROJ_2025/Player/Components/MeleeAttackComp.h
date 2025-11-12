
#pragma once

#include "CoreMinimal.h"
#include "AttackComponentBase.h"
#include "MeleeAttackComp.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UMeleeAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UMeleeAttackComp();

	virtual void StartAttack() override;

protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack() override;

	virtual void SetCurrentAnimIndex();

	UFUNCTION(Server, Reliable)
	virtual void Server_PlayAttackAnim();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PlayAttackAnim();

	virtual void PlayAttackAnim();

	virtual float GetCurrentAnimLength();
	
	virtual void CheckForCollisionWithEnemy();

	virtual void Sweep(FVector SweepLocation);

	bool bIsFirstAttackAnimSet = false;

	int32 CurrentAttackAnimIndex = 0;

	FTimerHandle SweepTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "First Attack Comp Animations")
	TArray<UAnimMontage*> AttackAnims;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Radius")
	float AttackRadius = 50.f;
};
