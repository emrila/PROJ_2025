// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "CactusCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API ACactusCharacter : public AEnemyBase
{
	GENERATED_BODY()

	

public:

	UFUNCTION(Server, Reliable)
	void Server_ShootProjectile(FVector SpawnLocation, FRotator SpawnRotation);

	UFUNCTION(Server, Reliable)
	void Server_SpawnSpikeExplosion(FVector SpawnLocation, FRotator SpawnRotation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> SpikeExplosionClass;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsBurrowing;
	
protected:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	
};
