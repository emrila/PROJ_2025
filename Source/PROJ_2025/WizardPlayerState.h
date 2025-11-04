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
	UPROPERTY(BlueprintReadOnly, Category = "PlayerInfo")
	FString SteamID;

	UPROPERTY(BlueprintReadOnly, Category = "PlayerInfo")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "PlayerInfo")
	int32 PlayerLevel;
};
