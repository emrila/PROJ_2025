
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
	
	UFUNCTION(Server, Reliable)
	virtual void Server_SpawnShield();
	
	/*UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_SpawnShield();*/

	virtual float GetAttackCooldown() const override;

	virtual float GetDamageAmount() const override;

	virtual float GetDurability();

	virtual float GetRecoveryRate();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Server, Reliable)
	virtual void Server_ActivateShield();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_ActivateShield();

	UFUNCTION(Server, Reliable)
	virtual void Server_DeactivateShield();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_DeactivateShield();

	virtual void StartAttackCooldown();

protected:
	virtual void BeginPlay() override;
	
	bool bIsShieldActive = false;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AShield> ShieldClass;
	
	UPROPERTY(Replicated)
	AShield* CurrentShield;

	FTimerHandle DurabilityTimerHandle;
	FTimerHandle RecoveryTimerHandle;

	float TheCurrentDurability = 0.f;
	float BaseDurability = 200.f;

	float BaseRecoveryRate = 1.f;
};
