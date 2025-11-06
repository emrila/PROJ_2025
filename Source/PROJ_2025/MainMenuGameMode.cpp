// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"


#include "PlayerLoginSystem.h"

#include "Blueprint/UserWidget.h"

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UPlayerLoginSystem* LoginSystem = GetGameInstance()->GetSubsystem<UPlayerLoginSystem>())
	{
		LoginSystem->OnUsernameRequired.AddDynamic(this, &AMainMenuGameMode::ShowUsernameInputWidget);
		LoginSystem->OnLoginSuccess.AddDynamic(this, &AMainMenuGameMode::ShowMainMenu);
		LoginSystem->StartLoginProcess();
	}
}

void AMainMenuGameMode::ShowUsernameInputWidget()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		UsernameInputWidget = CreateWidget<UUserWidget>(PC, UsernameInputWidgetClass);
		UsernameInputWidget->AddToViewport();
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(UsernameInputWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}


void AMainMenuGameMode::ShowMainMenu()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UsernameInputWidget)
		{
			UsernameInputWidget->RemoveFromParent();
		}
		MainMenuWidget = CreateWidget<UUserWidget>(PC, MainMenuWidgetClass);
		MainMenuWidget->AddToViewport();

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(MainMenuWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		
	}
}



