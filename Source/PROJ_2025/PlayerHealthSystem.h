// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerHealthSystem.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API UPlayerHealthSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	UPROPERTY()
	float MaxHealth = 100;
	
	UPROPERTY()
	float Health = 100;

public:
	UFUNCTION(BlueprintCallable)
	void DamageHealth(float DamageAmount);

	UFUNCTION(BlueprintCallable)
	void RestoreHealth(float RestoreAmount);
	
	UFUNCTION(BlueprintCallable)
	void SetMaxHealth(float HealthAmount);

	UFUNCTION(BlueprintCallable)
	void SetHealth(float HealthAmount);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetHealthPercent() const;
};
