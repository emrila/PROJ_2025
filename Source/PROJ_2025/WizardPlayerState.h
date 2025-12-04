// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WizardPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API AWizardPlayerState : public APlayerState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintReadOnly, Category = "PlayerInfo")
	FString SteamID;

	UPROPERTY(BlueprintReadOnly, Category = "PlayerInfo")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "PlayerInfo")
	int32 PlayerLevel;

	UFUNCTION(Server, Reliable)
	void AddDamageTaken(float Damage);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetDamageTaken();

	UFUNCTION(Server, Reliable)
	void AddDamageDealt(float Damage);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetDamageDealt();

protected:
	UPROPERTY(Replicated)
	float DamageTaken = 0.f;

	UPROPERTY(Replicated)
	float DamageDealt = 0.f;
};
