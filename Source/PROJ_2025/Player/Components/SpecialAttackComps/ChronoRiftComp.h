
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
	
	UFUNCTION(BlueprintCallable)
	virtual float GetAttackRadius();

	UFUNCTION(BlueprintCallable)
	virtual float GetDamageAmount() const override;
	

protected:
	virtual void BeginPlay() override;
	
	virtual void PerformAttack() override;
	
	virtual void TryLockingTargetArea();
	
	virtual void OnStartLockingTargetArea(const FInputActionInstance& InputActionInstance);
	
	virtual void OnTargetAreaLocked(const FInputActionInstance& InputActionInstance);
	
	virtual void PrepareForLaunch();
	
	virtual void ResetAttackCooldown() override;
	
	void PerformLaunch();
	
	UFUNCTION(Server, Reliable)
	virtual void Server_PerformLaunch();
	
	void SpawnChronoRiftZone();
	
	UFUNCTION(Server, Reliable)
	virtual void Server_SetTargetAreaCenter(const FVector& TargetCenter);

	virtual void SetIndicatorHidden(const bool bIsHidden);
	
	void UpdateIndicatorScale();

	virtual float GetChronoDuration() const;

	virtual float GetAttackCooldown() const override;


	UFUNCTION(Server, Reliable)
	void Server_SetIndicatorRadius(const float NewRadius);
	
	UPROPERTY()
	AActor* LovesMesh;
	
	UPROPERTY(editAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ChronoRiftIndicatorClass;
	
	UPROPERTY(Replicated)
	FVector TargetAreaCenter;
	
	float TargetAreaRadius = 400.f;
	
	UPROPERTY(Replicated)
	float IndicatorRadius = 400.f;
	
	bool bIsLockingTargetArea = false;

	//Not changed by the upgrade system
	float LockOnRange = 30000.f;
	
	float ChronoDuration = 4.f;
	
	float EnemyTimeDilationFactor = 0.3f;

	UPROPERTY(Replicated)
	AChronoRiftZone* CurrentChronoRiftZone;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AChronoRiftZone> ChronoRiftZoneClass;
	
	UFUNCTION(Server, Reliable)
	void Server_Debuging();
};
