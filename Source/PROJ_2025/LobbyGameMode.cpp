// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "WizardPlayerState.h"
#include "OnlineSubsystemUtils.h"
#include "WizardGameState.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();
	//ShowLobbyWidget();
	
}

void ALobbyGameMode::ShowLobbyWidget()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		LobbyWidget = CreateWidget<UUserWidget>(PC, LobbyWidgetClass);
		LobbyWidget->AddToViewport();
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void ALobbyGameMode::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameMode, PlayerCount);
}


void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	
	
	if (!HasAuthority()) 
	{
		return;
	}
	AWizardGameState* GS = GetWorld()->GetGameState<AWizardGameState>();
	GS->CurrentPlayerCount++;


	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,              
			16.f,             
			FColor::Black,   
			FString::Printf(TEXT("LOGIN DETECTED"))
		);
	}
	AWizardPlayerState* PS = Cast<AWizardPlayerState>(NewPlayer->PlayerState);
	
	if (!PS) return;
	if (IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld()))
	{
		if (IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface())
		{
			if (TSharedPtr<const FUniqueNetId> UniqueId = NewPlayer->PlayerState->GetUniqueId().GetUniqueNetId())
			{
				FString SteamID = UniqueId->ToString();
				PS->SteamID = SteamID;
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Player joined with ID %s"), *PS->SteamID);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,              
			16.f,             
			FColor::Black,   
			FString::Printf(TEXT("Player joined with ID %s"), *PS->SteamID)
		);
	}
}
