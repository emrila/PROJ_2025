
#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AttackComponentBase.h"
#include "ShieldAttackComp.generated.h"


struct FInputActionInstance;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UShieldAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UShieldAttackComp();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction) override;
	
	virtual void StartAttack() override;

	virtual void SpawnShield();

	virtual float GetAttackCooldown() const override;

	virtual float GetDamageAmount() const override;

	virtual float GetDurability();

	virtual float GetRecoveryRate();

	virtual void ActivateShield();

	virtual void DeactivateShield();

	virtual void StartAttackCooldown();
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_StartAttackCooldown();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnPlayerDied(bool bNewIsAlive);
	
	virtual void ResetAttackCooldown() override;
	
	bool bIsShieldActive = false;
	
	bool bShouldHandleSprint = false;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AShield> ShieldClass;
	
	UPROPERTY(Replicated)
	AShield* CurrentShield;

	FTimerHandle DurabilityTimerHandle;
	FTimerHandle RecoveryTimerHandle;

	float TheCurrentDurability = 0.f;
	float BaseDurability = 200.f;

	float BaseRecoveryRate = 1.f;
	
	float CurrentMoveSpeed = 0.f;
	
	void HandleOwnerMovement(const float NewMoveSpeed);
	
	UFUNCTION(Server, Reliable)
	void Server_HandleOwnerMovement(const float NewMoveSpeed);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HandleOwnerMovement(const float NewMoveSpeed);
};
