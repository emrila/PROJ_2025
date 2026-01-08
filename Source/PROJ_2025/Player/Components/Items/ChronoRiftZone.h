
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChronoRiftZone.generated.h"

class UNiagaraComponent;
class APlayerCharacterBase;
class ACharacter;
class UNiagaraSystem;
class USphereComponent;

UCLASS()
class PROJ_2025_API AChronoRiftZone : public AActor
{
	GENERATED_BODY()

public:
	AChronoRiftZone();
	
	void SetOwnerCharacter(APlayerCharacterBase* NewOwnerCharacter);

	void SetInitialValues(
		const float NewRadius,
		const float NewLifetime,
		const float NewDamageAmount
		);

protected:
	virtual void BeginPlay() override;
	void MakeInitialSphereSweep();
	void TickDamage();
	void HandleDestroySelf();
	
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep,
		const FHitResult& SweepResult
		);
	
	void HandleOverlapBegin(AActor* OtherActor);
	
	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex
		);
	
	void HandleOverlapEnd(AActor* OtherActor);
	
	void RequestSpawnEffect();
	void SpawnEffect(const float NewChronoDuration, const float BaseRadius, const float CurrentRadius) const;

	UFUNCTION(Server, Reliable)
	void Server_SpawnEffect(const float NewLifeTime, const float BaseRadius, const float CurrentRadius);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnEffect(const float NewLifeTime, const float BaseRadius, const float CurrentRadius);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereComponent;
	
	float Radius = 400;
	float InitialRadius = 400;
	
	float Lifetime = 4.f;
	
	float DamageAmount = 2;
	
	float EnemyTimeDilationFactor = 0.3f;
	
	FTimerHandle TickDamageTimerHandle;
	
	UPROPERTY()
	APlayerCharacterBase* OwnerCharacter;
	
	UPROPERTY()
	TSet<AActor*> OverlappedEnemies;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UNiagaraSystem* ChronoRiftEffect;
};