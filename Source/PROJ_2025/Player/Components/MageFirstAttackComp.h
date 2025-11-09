#pragma once

#include "CoreMinimal.h"
#include "AttackComponentBase.h"
#include "MageFirstAttackComp.generated.h"

class AMageProjectile;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UMageFirstAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UMageFirstAttackComp();

	virtual void StartAttack() override;

protected:
	virtual void BeginPlay() override;
	
	virtual void PerformAttack() override;

	UFUNCTION(Server, Reliable)
	virtual void Server_SpawnProjectile(const FVector SpawnLocation);

	virtual FTransform GetProjectileTransform();

	virtual FVector GetProjectileSpawnLocation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMageProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ProjectileSpawnSocketName; // = "spine_05Socket";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector SpawnLocationOffset;
};
