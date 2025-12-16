
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
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetImpactParticle(UNiagaraSystem* Particles);
	
	UFUNCTION(Server, Reliable)
	void Server_SetDamageAmount(const float NewDamageAmount);
	
	UFUNCTION(Server, Reliable)
	void Server_SetProjectileSpeed(const float NewProjectileSpeed);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
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

	UPROPERTY(Replicated)
	float ProjectileSpeed = 3000.0f;
	
	float LifeTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	UNiagaraSystem* ImpactParticles;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	float PiercingAmount = 2.f;
	
	UPROPERTY(Replicated)
	TArray<AActor*> HitEnemies;
	
	UPROPERTY(Replicated)
	FVector CurrentScale;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Scale", Replicated)
	float ScaleFactor = 4.f;
	
	UPROPERTY(Replicated)
	float AlphaElapsed = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Scale", Replicated)
	float ScaleDuration = 0.3f;
};
