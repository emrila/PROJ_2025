#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AttackComponentBase.h"
#include "DashAttackComp.generated.h"


class AShadowStrikeRibbon;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCanRecast);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecast);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UDashAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UDashAttackComp();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
						   FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(BlueprintAssignable)
	FOnCanRecast OnCanRecast;
	
	UPROPERTY(BlueprintAssignable)
	FOnRecast OnRecast;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnPreAttack(const FInputActionInstance& InputActionInstance) override;
	virtual void OnStartAttack(const FInputActionInstance& InputActionInstance) override;
	
	virtual void StartAttack() override;
	virtual void PerformAttack() override;
	
	void RequestDash(const FVector& NewTargetLocation);
	void Dash(const FVector& NewTargetLocation);
	
	UFUNCTION(Server, Reliable)
	void Server_Dash(const FVector& NewTargetLocation);
	
	void PerformSweep();
	
	virtual float GetCooldownDuration() override;
	
	void SetShouldEverRecast(const bool bNewShouldEverRecast);
	
	UFUNCTION(Server, Reliable)
	void Server_SetShouldEverRecast(const bool bNewShouldEverRecast);
	
	void SetShouldRecast(const bool bNewShouldRecast);
	
	UFUNCTION(Server, Reliable)
	void Server_SetShouldRecast(const bool bNewShouldRecast);
	
	void SetDidRecast(const bool bNewDidRecast);
	
	UFUNCTION(Server, Reliable)
	void Server_SetDidRecast(const bool bNewDidRecast);
	
	UFUNCTION(Server, Reliable)
	void Server_SetIsDashing(const bool bNewIsDashing);
	
	FVector GetTargetLocation() const;
	
	void HandlePostAttackState() const;
	
	virtual void Reset() override;
	
	UFUNCTION()
	void OnRepShouldRecast();
	
	UFUNCTION()
	void OnRepWentThroughShield();
	
	float DashRange = 1000.f;
	float DashDuration = 0.2f;
	float RecastDuration = 3.0f;
	float ShieldInvincibilityDuration = 5.f;
	
	UPROPERTY(Replicated)
	float DashElapsed = 0.0f;
	
	bool bIsLockingTargetLocation = false;
	
	UPROPERTY(Replicated)
	bool bIsDashing = false;
	
	bool bShouldEverRecast = true;
	
	UPROPERTY(ReplicatedUsing = OnRepShouldRecast)
	bool bShouldRecast = false;
	
	UPROPERTY(Replicated)
	bool bDidRecast = false;
	
	UPROPERTY(ReplicatedUsing = OnRepWentThroughShield)
	bool bWentThroughShield = false;
	
	UPROPERTY(Replicated)
	FVector StartLocation;
	
	UPROPERTY(Replicated)
	FVector TargetLocation;
	
	FTimerHandle RecastTimer;
	FTimerHandle IFrameTimer;
	
	UPROPERTY()
	AShadowStrikeRibbon* Ribbon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TSubclassOf<AShadowStrikeRibbon> RibbonClass;
};
