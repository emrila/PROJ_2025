#pragma once

#include "CoreMinimal.h"
#include "../AttackComponentBase.h"
#include "RangeAttackComp.generated.h"

class AMageProjectile;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API URangeAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	URangeAttackComp();

	virtual void StartAttack() override;

protected:
	virtual void BeginPlay() override;
	
	virtual void PerformAttack() override;

	UFUNCTION(Server, Reliable)
	virtual void Server_SpawnProjectile(const FTransform SpawnTransform);
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_SpawnProjectile(const FTransform SpawnTransform);

	virtual FTransform GetProjectileTransform();

	virtual float GetAttackCooldown() const override;

	virtual float GetDamageAmount() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMageProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileOffsetDistanceInFront = 120.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackAnimation;
};
