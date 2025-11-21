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

	virtual FTransform GetProjectileTransform();

	virtual FRotator GetProjectileSpawnRotation();

	virtual FVector GetProjectileSpawnLocation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMageProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ProjectileSpawnSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SpawnLocationOffset;
};
