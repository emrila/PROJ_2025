
#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "GameFramework/Actor.h"
#include "MageProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PROJ_2025_API AMageProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMageProjectile();

	UFUNCTION(BlueprintCallable)
	void SetImpactParticle(UNiagaraSystem* Particles);
	
	UFUNCTION(Server, Reliable)
	void Server_SetDamageAmount(const float NewDamageAmount);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION(blueprintCallable)
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
	
	UPROPERTY(Replicated)
	float DamageAmount = 10.0f;

	float GetDamageAmount() const { return DamageAmount; }
	void SetDamageAmount(const float Value) { DamageAmount = Value; }
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileSpeed = 1000.0f;

	float GetProjectileSpeed() const { return ProjectileSpeed; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LifeTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	UNiagaraSystem* ImpactParticles;

	
	
};
