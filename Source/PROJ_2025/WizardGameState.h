// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
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
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	float Health;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float HealthPercent = 1;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void DamageHealth(float DamageAmount);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void RestoreHealth(float RestoreAmount);
	
	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void SetMaxHealth(float HealthAmount);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable)
	void SetHealth(float HealthAmount);


	
};
