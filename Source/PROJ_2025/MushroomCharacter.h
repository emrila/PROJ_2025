// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "MushroomCharacter.generated.h"

UCLASS()
class PROJ_2025_API AMushroomCharacter : public AEnemyBase
{
	GENERATED_BODY()

public:
	AMushroomCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Multicast_Jump(float Angle, FRotator RotationToPlayer, float JumpStrength, float ForwardStrength);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RotateSpeed;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool AttackIsOnCooldown = false;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsAttacking;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool Bounce;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsRunning = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool SlowRunning = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool MeleeRun = false;
	
	UFUNCTION(BlueprintCallable)
	void StartSmoothRotationTo(FRotator TargetRotation, float Speed);


	UPROPERTY(Replicated, BlueprintReadWrite)
	bool WarmingUp;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool Projectile = false;

	virtual void HandleHit(struct FDamageEvent const& DamageEvent, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimerHandle RotationTimerHandle;

	
private:
	
	void UpdateSmoothRotation();

	FRotator DesiredRotation;
	float RotationSpeed;
};
