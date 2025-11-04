// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ShowMainMenu();

	UFUNCTION()
	void ShowUsernameInputWidget();



	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> UsernameInputWidgetClass;


private:
	UPROPERTY()
	UUserWidget* MainMenuWidget;
	
	UPROPERTY()
	UUserWidget* UsernameInputWidget;

};


