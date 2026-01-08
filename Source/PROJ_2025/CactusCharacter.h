// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "CactusCharacter.generated.h"

/**
 * 
 */

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttackAnimNotify, FVector, SpawnLocation);

UCLASS()
class PROJ_2025_API ACactusCharacter : public AEnemyBase
{
	GENERATED_BODY()

public:

	UFUNCTION(Server, Reliable)
	void Server_ShootProjectile(const FVector& NewTargetLocation);

	UFUNCTION(Server, Reliable)
	void Server_SpawnSpikeExplosion(FVector SpawnLocation, FRotator SpawnRotation);
	
	UFUNCTION(Server, Reliable)
	void Server_HandleOnAttackAnimNotify(const FVector SpawnLocation);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> SpikeExplosionClass;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsBurrowing;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpSpeed = 1.5f;
	
	UPROPERTY(Replicated)
	FVector CurrentProjectileSocketLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FRotator ProjectileSpawnRotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FVector TargetActorLocation;
	
	UPROPERTY(Replicated)
	bool bIsPlayingAnimation = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackAnim;
	
	UPROPERTY(BlueprintCallable)
	FOnAttackAnimNotify OnAttackAnimNotify;
	
	FTimerHandle AnimationTimer;
	FTimerHandle InitialAttackTimer;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	
};
