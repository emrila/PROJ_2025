#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AttackComponentBase.h"
#include "DashAttackComp.generated.h"


class AShadowStrikeRibbon;
struct FInputActionInstance;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UDashAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UDashAttackComp();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction) override;
	
	virtual void StartAttack() override;

protected:
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void PerformAttack() override;
	
	virtual void OnPrepareForAttack(const FInputActionInstance& ActionInstance);
	
	virtual void OnStartAttack(const FInputActionInstance& ActionInstance);
	
	//virtual void OnAttackCanceled(const FInputActionInstance& ActionInstance);
	
	void PrepareForAttack();
	
	void TryLockingTargetLocation();
	
	UFUNCTION(Server, Reliable)
	void Server_SetCanDash(const bool Value);
	
	UFUNCTION(Server, Reliable)
	void Server_SetStartAndTargetLocation(const FVector& NewStartLocation, const FVector& NewLockedLocation);
	
	UFUNCTION(Server, Reliable)
	void Server_SetWentThroughShield(const bool Value);
	
	UFUNCTION(Server, Reliable)
	void Server_SetShouldRecast(const bool bNewShouldRecast);
	
	UFUNCTION(Server, Reliable)
	void Server_SetDidRecast(const bool bNewDidRecast);
	
	UFUNCTION(Server, Reliable)
	void Server_SetIsDashing(const bool bNewIsDashing);
	
	UFUNCTION(Server, Reliable)
	void Server_SetHasLockedTargetLocation(const bool bNewHasLockedTargetLocation);
	
	UFUNCTION(Server, Reliable)
	void Server_PerformSweep();
	
	void Dash();
	
	UFUNCTION(Server, Reliable)
	void Server_Dash();
	
	void HandlePostAttackState();
	
	virtual void ResetAttackCooldown() override;
	
	virtual float GetAttackCooldown() const override;
	
	virtual float GetDamageAmount() const override;
	
	UPROPERTY(Replicated)
	AShadowStrikeRibbon* Ribbon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<AShadowStrikeRibbon> RibbonClass;
	
	UPROPERTY(Replicated)
	FVector TargetLocation;
	
	UPROPERTY(Replicated)
	FVector StartLocation;
	
	//
	float DashRange = 1000.f;
	
	float DashDuration = 0.2f;
	
	float RecastDuration = 3.0f;
	
	float DashElapsed = 0.0f;
	
	UPROPERTY(Replicated)
	bool bShouldRecast = false;
	
	UPROPERTY(Replicated)
	bool bDidRecast = false;
	
	UPROPERTY(Replicated)
	bool bWentThroughShield = false;
	
	UPROPERTY(Replicated)
	bool bCanDash = false;
	
	UPROPERTY(Replicated)
	bool bIsDashing = false;
	
	UPROPERTY(Replicated)
	bool bHasLockedTargetLocation = false;
	
	FTimerHandle DashTimer;
	FTimerHandle RecastTimer;
	FTimerHandle IFrameTimer;
};
