#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AttackComponentBase.h"
#include "ShieldAttackComp.generated.h"


class AShield;

//UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDurabilityChanged, float, CurrentDurability, float, MaxDurability);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UShieldAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UShieldAttackComp();
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction) override;
	void HandleCooldown();
	
	UFUNCTION(BlueprintCallable)
	float GetDurability() const;
	
	UPROPERTY(BlueprintAssignable)
	FOnDurabilityChanged OnDurabilityChanged;
	
	void HandleOnDurabilityChanged(const float NewDurability) const;

protected:
	virtual void BeginPlay() override;
	virtual void StartAttack() override;
	
	void RequestToggleShield(const bool bShouldActivate);
	void ToggleShield(const bool bShouldActivate);
	
	UFUNCTION(Server, Reliable)
	void Server_ToggleShield(const bool bShouldActivate);
	
	float GetRecoveryRate() const;
	
	UFUNCTION()
	void HandlePlayerDeath(const bool bNewIsAlive);
	
	void HandleOwnerMovement(const bool bShouldSlowDown);
	
	UFUNCTION(Client, Reliable)
	void Client_HandleOwnerMovement(const float NewMoveSpeed);
	
	UFUNCTION(Client, Reliable)
	void Client_HandleOnDurabilityChanged(const float NewDurability) const;
	
	virtual void Debug() override;
	
	bool bIsShieldActive = false;
	
	UPROPERTY()
	AShield* Shield;
	
	float BaseDurability = 200.f;
	float BaseRecoveryRate = 1.f;
	float MovementSpeedToStore = 0.f;
	float OwnerMovementSpeedMultiplier = 0.75f;
	
	FTimerHandle RecoveryTimerHandle;
	
};
