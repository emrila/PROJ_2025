
#pragma once

#include "CoreMinimal.h"
#include "../AttackComponentBase.h"
#include "ChronoRiftComp.generated.h"


class AChronoRiftZone;
class UChronoRiftDamageType;
class AEnemyBase;
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
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	virtual void PerformAttack() override;
	
	virtual void TryLockingTargetArea();
	
	virtual void OnStartLockingTargetArea(const FInputActionInstance& InputActionInstance);
	
	virtual void OnTargetAreaLocked(const FInputActionInstance& InputActionInstance);
	
	virtual void OnStartLockingCanceled(const FInputActionInstance& InputActionInstance);
	
	virtual void PrepareForLaunch();
	
	virtual void ResetAttackCooldown() override;
	
	UFUNCTION(Server, Reliable)
	virtual void Server_PerformLaunch(); 
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_PerformLaunch();
	
	UFUNCTION(Server, Reliable)
	virtual void Server_SetTargetAreaCenter(const FVector& TargetCenter);

	virtual void SetIndicatorHidden(bool bIsHidden);

	virtual float GetChronoDuration() const;

	virtual float GetAttackRadius() const;

	virtual float GetAttackCooldown() const override;

	virtual float GetDamageAmount() const override;
	
	UFUNCTION(Client, Reliable)
	void Client_SpawnChronoRiftIndicator();
	
	UPROPERTY()
	AActor* LovesMesh;
	
	UPROPERTY(editAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ChronoRiftIndicatorClass;
	
	UPROPERTY(Replicated)
	FVector TargetAreaCenter;
	
	float TargetAreaRadius = 400.f;
	
	bool bIsLockingTargetArea = false;

	//Not changed by the upgrade system
	float LockOnRange = 30000.f;
	
	float ChronoDuration = 4.f;
	
	float EnemyTimeDilationFactor = 0.3f;

	UPROPERTY(Replicated)
	AChronoRiftZone* CurrentChronoRiftZone;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AChronoRiftZone> ChronoRiftZoneClass;
};
