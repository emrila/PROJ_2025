#pragma once

#include "CoreMinimal.h"
#include "../AttackComponentBase.h"
#include "ShadowStrikeAttackComp.generated.h"


class UNiagaraSystem;
struct FInputActionInstance;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UShadowStrikeAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UShadowStrikeAttackComp();

	virtual void StartAttack() override;
	
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction) override;
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void SetKilledTarget(const bool Value) { bKilledTarget = Value; }

protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack() override;
	
	virtual void OnPrepareForAttack(const FInputActionInstance& ActionInstance);
	
	virtual void OnLockedTarget(const FInputActionInstance& ActionInstance);
	
	virtual void OnAttackCanceled(const FInputActionInstance& ActionInstance);
	
	virtual void PrepareForAttack();

	virtual void TryLockingTargetOrLocation();
	
	virtual void TryLockingTarget(FVector StartLocation, FVector EndLocation);

	virtual void TryLockingLocation(FVector StartLocation, FVector EndLocation);
	
	UFUNCTION(Server, Reliable)
	void Server_SetLockedTarget(AActor* Target);

	UFUNCTION(Server, Reliable)
	void Server_SetWentThroughShield(const bool Value);

	UFUNCTION(Server, Reliable)
	void Server_SetLockedLocation(FVector Location, FVector SweepStart);
	
	virtual void HandlePreAttackState();
	
	virtual void HandlePostAttackState();
	
	UFUNCTION(Server, Reliable)
	virtual void Server_TeleportPlayer();
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_TeleportPlayer(
		const FVector& TeleportLocation);

	UFUNCTION(Server, Reliable)
	virtual void Server_PerformSweep();

	/*virtual void ResetAttackCooldown() override;

	virtual float GetAttackCooldown() const override;

	virtual float GetDamageAmount() const override;*/

	virtual float GetAttackRange() const;

	//Handle target
	bool bHasLockedTarget = false;

	UPROPERTY()
	AActor* LockedTarget;

	FVector LockedLocation;

	FVector SweepStartLocation;
	
	float LockOnRange = 2000.f;

	float MinimumDistanceToTarget = 200.f;
	
	FTimerHandle LockedTargetTickTimer;
	
	bool bKilledTarget = false;

	bool bCanTeleport = false;

	bool bWentThroughShield = false;

	//Handle attack properties
	float OffsetDistanceBehindTarget = 100.f;

	float AcceptableAngelDegrees = 10.f;
	
	float StrikeDuration = 0.5f;
	
	//float StrikeDelay = 1.f;
	
	//Handle player teleport
	float TeleportDelay = 0.2f;
	
	FTimerHandle PlayerTeleportTimerHandle;
	
	//Handle camera interpolation
	float CameraInterpDistanceBehind = 500.f;
	
	float CameraInterpDuration = 0.3f;
	
	float CameraInterpDelay = 0.5f;
	
	//VFX
	FVector DisappearLocation;
	FVector AppearLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* DisappearEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* AppearEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	UNiagaraSystem* TeleportEffect;

	FTimerHandle PlayerIFrameTimer;
};
