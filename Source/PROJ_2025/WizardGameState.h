// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "GameFramework/GameState.h"
#include "WizardGameState.generated.h"

/**
 * 
 */
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

	UFUNCTION(BlueprintImplementableEvent)
	void OnRep_Health();

	UPROPERTY(Replicated, BlueprintReadOnly)
	float HealthPercent = 1;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void PlayerEnteredStartDungeon(bool Entered);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void DamageHealth(float DamageAmount);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void RestoreHealth(float RestoreAmount);
	
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void SetMaxHealth(float HealthAmount);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void SetHealth(float HealthAmount);

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool MageSelected = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool TankSelected = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool RogueSelected = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	TArray<UWidget*> SelectionWidgets;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 CurrentPlayerCount;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 PlayersInStartDungeon;
	
};
