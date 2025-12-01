// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FindSessionsCallbackProxy.h"
#include "GameFramework/GameModeBase.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FLobbyInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString LobbyName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString HostID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Password;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int MaxPlayers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int PlayerCount;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FBlueprintSessionResult SessionResult;
};

UCLASS()

class PROJ_2025_API ALobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	UFUNCTION()
	void ShowLobbyWidget();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated, BlueprintReadOnly)
	int PlayerCount = 0;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	
private:
	
	UPROPERTY()
	UUserWidget* LobbyWidget;
};
