// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "GameFramework/GameState.h"
#include "WizardGameState.generated.h"

/**
 * 
 */

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuddenDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuddenDeathEnd);
UCLASS()
class PROJ_2025_API AWizardGameState : public AGameStateBase
{
	GENERATED_BODY()

	

public:

	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, Replicated, BlueprintReadOnly)
	float MaxHealth;
	
	UPROPERTY(Replicated, BlueprintReadOnly, ReplicatedUsing = OnRep_Health)
	float Health;

	UFUNCTION()
	void OnRep_Health() const;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float HealthPercent = 1;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void PlayerEnteredStartDungeon(bool Entered);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void DamageHealth(float DamageAmount);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void RestoreHealth(float RestoreAmount);
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetMaxHealth(float HealthAmount);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetHealth(float HealthAmount);

	UPROPERTY(Replicated, BlueprintReadWrite)
	TArray<UWidget*> SelectionWidgets;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 CurrentPlayerCount;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 PlayersInStartDungeon;

	UPROPERTY(Replicated)
	float LifeStealMultiplier = 1.f;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChangedDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnSuddenDeath;
	
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnSuddenDeathEnd;
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SuddenDeath();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndSuddenDeath();
	
};
