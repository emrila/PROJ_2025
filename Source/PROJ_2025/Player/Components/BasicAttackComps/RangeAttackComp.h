#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AttackComponentBase.h"
#include "RangeAttackComp.generated.h"


class AMageProjectile;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API URangeAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	URangeAttackComp();
	
	float GetProjectileSpeed() const;

protected:
	virtual void StartAttack() override;
	virtual void PerformAttack() override;
	
	void RequestSpawnProjectile();
	
	void SpawnProjectile(const FTransform& SpawnTransform) const;
	
	UFUNCTION(Server, Reliable)
	void Server_SpawnProjectile(const FTransform& SpawnTransform);
	
	FTransform GetProjectileTransform() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMageProjectile> ProjectileClass;
	
	float ProjectileOffsetDistanceInFront = 250.f;
	float DefaultProjectileSpeed = 4000.f;
};
