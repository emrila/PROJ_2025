#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MageProjectile.generated.h"

class UNiagaraSystem;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class PROJ_2025_API AMageProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMageProjectile();
	
	virtual void Tick(float DeltaTime) override;
	
	void SetDamageAmount(const float NewDamageAmount);
	void SetProjectileSpeed(const float NewProjectileSpeed) const;

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
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* EnemyCollisionComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USphereComponent* WorldStaticCollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* ProjectileMovementComponent;
	
	float DamageAmount = 10.0f;
	
	float ProjectileSpeed = 3000.0f;
	
	float LifeTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* ImpactParticles;
	
	float PiercingAmount = 2.f;
	
	UPROPERTY()
	TArray<AActor*> HitEnemies;
	
	FVector CurrentScale;
	
	float ScaleFactor = 4.f;
	
	float AlphaElapsed = 0.f;
	
	float ScaleDuration = 0.3f;
};
