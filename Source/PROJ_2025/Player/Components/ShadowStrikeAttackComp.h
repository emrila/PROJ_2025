#pragma once

#include "CoreMinimal.h"
#include "AttackComponentBase.h"
#include "ShadowStrikeAttackComp.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UShadowStrikeAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UShadowStrikeAttackComp();

	virtual void StartAttack() override;

protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack() override;
	
	UFUNCTION(Server, Reliable)
	void Server_SetLockedTarget(AActor* Target);
	
	virtual void HandlePreAttackState();
	
	virtual void HandlePostAttackState();
	
	UFUNCTION(Server, Reliable)
	virtual void Server_TeleportPlayer();
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_TeleportPlayer(const FTransform BehindTransform);
	
	virtual void TryLockingTarget();

	virtual FTransform GetTransformBehindLockedTarget() const;

	virtual void ResetAttackCooldown() override;

	bool bIsLockingTarget = false;

	UPROPERTY()
	AActor* LockedTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnRange = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OffsetDistanceBehindTarget = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDuration = 3.f;
};
