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
	
	virtual void HandlePlayerCamera();

	virtual void TryLockingTarget();

	virtual FTransform GetLocationBehindLockedTarget() const;

	virtual void ResetAttackCooldown() override;

	bool bIsLockingTarget = false;

	UPROPERTY()
	AActor* LockedTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnRange = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OffsetDistanceBehindTarget = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDuration = 2.f;
};
