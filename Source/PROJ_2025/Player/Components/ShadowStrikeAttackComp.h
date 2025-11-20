#pragma once

#include "CoreMinimal.h"
#include "AttackComponentBase.h"
#include "ShadowStrikeAttackComp.generated.h"


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
	
	virtual void TryLockingTarget();
	
	UFUNCTION(Server, Reliable)
	void Server_SetLockedTarget(AActor* Target);
	
	virtual void HandlePreAttackState();
	
	virtual void HandlePostAttackState();
	
	UFUNCTION(Server, Reliable)
	virtual void Server_TeleportPlayer();
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_TeleportPlayer(
		const FVector& TeleportLocation);

	virtual void ResetAttackCooldown() override;
	
	virtual void KillTarget(AActor* Target);

	//Handle target
	bool bIsLockingTarget = false;
	
	bool bHasLockedTarget = false;

	UPROPERTY()
	AActor* LockedTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnRange = 2000.f;
	
	FTimerHandle LockedTargetTickTimer;
	
	bool bKilledTarget = false;

	//Handle attack properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OffsetDistanceBehindTarget = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StrikeDuration = 3.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StrikeDelay = 1.f;
	
	//Handle camera interpolation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraInterpDistanceBehind = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraInterpHeight = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraInterpDuration = 0.35f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraInterpDelay = 1.f;
};
