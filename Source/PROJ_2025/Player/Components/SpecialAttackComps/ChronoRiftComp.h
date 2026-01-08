#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AttackComponentBase.h"
#include "ChronoRiftComp.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UChronoRiftComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UChronoRiftComp();
	
	UFUNCTION(BlueprintCallable)
	float GetAttackRadius() const;

protected:
	virtual void BeginPlay() override;
	
	virtual void OnPreAttack(const FInputActionInstance& InputActionInstance) override;
	virtual void OnStartAttack(const FInputActionInstance& InputActionInstance) override;
	
	virtual void StartAttack() override;
	virtual void PerformAttack() override;
	
	void UpdateIndicatorScale() const;
	void SetIndicatorHidden(const bool Value) const;
	
	void RequestSpawnChronoRiftZone();
	void SpawnChronoRiftZone(const FVector& SpawnLocation) const;
	
	UFUNCTION(Server, Reliable)
	void Server_SpawnChronoRiftZone(const FVector& SpawnLocation);
	
	FVector GetIndicatorActorLocation() const;
	
	UPROPERTY()
	AActor* IndicatorActor;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> IndicatorActorClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ChronoRiftZoneClass;
	
	float TargetAreaRadius = 400.f;
	float ChronoDuration = 4.f;
	float LockOnRange = 30000.f;
	
	virtual void Debug() override;
};
