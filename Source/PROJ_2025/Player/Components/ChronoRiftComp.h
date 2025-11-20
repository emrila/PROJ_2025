
#pragma once

#include "CoreMinimal.h"
#include "AttackComponentBase.h"
#include "ChronoRiftComp.generated.h"


struct FInputActionInstance;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UChronoRiftComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UChronoRiftComp();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction) override;
	
	virtual void StartAttack() override;

protected:
	virtual void BeginPlay() override;
	
	virtual void PerformAttack() override;
	
	virtual void TryLockingTargetArea();
	
	virtual void OnStartLockingTargetArea(const FInputActionInstance& InputActionInstance);
	
	virtual void OnTargetAreaLocked(const FInputActionInstance& InputActionInstance);
	
	virtual void OnStartLockingCanceled(const FInputActionInstance& InputActionInstance);
	
	virtual void PrepareForLaunch();
	
	UFUNCTION(Server, Reliable)
	virtual void Server_PerformLaunch();
	
	UFUNCTION(Server, Reliable)
	virtual void Server_SetTargetAreaCenter(const FVector& TargetCenter);
	
	FVector TargetAreaCenter;
	
	float TargetAreaRadius = 1000.f;
	
	bool bIsLockingTargetArea = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockOnRange = 3000.f;
};
