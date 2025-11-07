// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpFwd.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerLoginSystem.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FPlayerProfile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString SteamID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Username;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Level;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Xp;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Gold;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLoginSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUsernameRequired);

UCLASS()
class PROJ_2025_API UPlayerLoginSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable)
	void StartLoginProcess();

	UFUNCTION(BlueprintCallable)
	void SendCreatePlayerRequest(const FString& Username);
	
	UFUNCTION(BlueprintCallable)
	const FPlayerProfile& GetProfile() const { return PlayerProfile; }

	UPROPERTY(BlueprintAssignable)
	FOnPlayerLoginSuccess OnLoginSuccess;
	FOnUsernameRequired OnUsernameRequired;
	
private:
	FPlayerProfile PlayerProfile;

	void SendCheckPlayerRequest(const FString& SteamID);
	
	void OnCheckPlayerResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnCreatePlayerResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) const;
};

