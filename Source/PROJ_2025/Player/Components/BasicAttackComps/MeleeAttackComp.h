#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AttackComponentBase.h"
#include "MeleeAttackComp.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UMeleeAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UMeleeAttackComp();
	
protected:
	virtual void StartAttack() override;
	virtual void PerformAttack() override;
	
	void RequestSweep();
	
	void Sweep(const FVector& SweepLocation);
	
	UFUNCTION(Server, Reliable)
	void Server_Sweep(const FVector& SweepLocation);
};
