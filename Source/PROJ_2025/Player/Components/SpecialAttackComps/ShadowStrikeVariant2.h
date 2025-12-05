#pragma once

#include "CoreMinimal.h"
#include "../AttackComponentBase.h"
#include "ShadowStrikeVariant2.generated.h"


class AShadowStrikeRibbon;
class UNiagaraSystem;
struct FInputActionInstance;

DECLARE_LOG_CATEGORY_EXTERN(ShadowStrikeLog, Log, All);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UShadowStrikeVariant2 : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UShadowStrikeVariant2();

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
	void Server_SetShouldRecast(const bool bNewShouldRecast);
	
	UFUNCTION(Server, Reliable)
	void Server_SetDidRecast(const bool BNewDidRecast);

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

	virtual void ResetAttackCooldown() override;

	virtual float GetAttackCooldown() const override;

	virtual float GetDamageAmount() const override;

	virtual float GetAttackRange() const;
	
	virtual void ResetRecast() {bShouldRecast = false;}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Properties")
	bool bShouldLockTarget = false;

	//Handle target
	bool bHasLockedTarget = false;

	UPROPERTY()
	AActor* LockedTarget;
	
	UPROPERTY()
	AShadowStrikeRibbon* Ribbon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<AShadowStrikeRibbon> RibbonClass;

	FVector LockedLocation;

	FVector SweepStartLocation;
	
	float LockOnRange = 1000.f;

	//Maybe not needed
	float MinimumDistanceToTarget = 400.f;
	
	FTimerHandle LockedTargetTickTimer;
	
	bool bKilledTarget = false;

	bool bCanTeleport = false;

	bool bWentThroughShield = false;
	
	bool bShouldRecast = false;
	
	bool bDidRecast = false;

	//Handle attack properties
	float OffsetDistanceBehindTarget = 100.f;

	float AcceptableAngelDegrees = 10.f;
	
	float StrikeDuration = 0.2f;
	
	float RecastDuration = 3.0f;
	
	//float StrikeDelay = 1.f;
	
	//Handle player teleport
	float TeleportDelay = 0.2f;
	
	FTimerHandle PlayerTeleportTimerHandle;
	FTimerHandle RecastTimerHandle;
	
	//Handle camera interpolation
	float CameraInterpDistanceBehind = 500.f;
	
	float CameraInterpDuration = 0.2f;
	
	float CameraInterpDelay = 0.f;
	
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

