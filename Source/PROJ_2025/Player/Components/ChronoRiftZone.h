
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChronoRiftZone.generated.h"

class ACharacter;
class UNiagaraSystem;
class USphereComponent;

UCLASS()
class PROJ_2025_API AChronoRiftZone : public AActor
{
	GENERATED_BODY()

public:
	AChronoRiftZone();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetOwnerCharacter(ACharacter* NewOwnerCharacter);

	virtual void SetInitialValues(
		const float NewRadius,
		const float NewLifetime,
		const float NewDamageAmount
		);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION(Server, Reliable)
	virtual void Server_SlowEnemy(AActor* Enemy);
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_SlowEnemy(AActor* Enemy);
	
	UFUNCTION(Server, Reliable)
	virtual void Server_ResetEnemy(AActor* Enemy);
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_ResetEnemy(AActor* Enemy);
	
	UFUNCTION(Server, Reliable)
	virtual void Server_ResetEnemiesPreEnd();
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_ResetEnemiesPreEnd();
	
	UFUNCTION()
	virtual void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	UFUNCTION()
	virtual void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex
		);

	UFUNCTION(Server, Reliable)
	virtual void Server_SpawnEffect();
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_SpawnEffect();  
	
	//UFUNCTION(Server, Reliable)
	virtual void Server_TickDamage();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* MeshComponent;
	
	float Radius = 200;
	
	float Lifetime = 4.f;
	
	float DamageAmount = 2;
	
	float EnemyTimeDilationFactor = 0.3f;
	
	FTimerHandle ResetEnemiesTimerHandle;
	FTimerHandle TickDamageTimerHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	UNiagaraSystem* ChronoRiftEffect;
	
	UPROPERTY()
	TSet<AActor*> EnemiesToGiveDamage;
	
	UPROPERTY()
	TSet<AActor*> EnemiesSLowedDown;
	
	UPROPERTY()
	ACharacter* OwnerCharacter;

	bool bIsInitialValuesSet = false;

	bool bIsFirstTick = true;
};