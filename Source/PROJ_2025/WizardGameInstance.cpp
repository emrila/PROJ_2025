// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardGameInstance.h"

#include "LobbyGameMode.h"
#include "LootPicker.h"
#include "RoomLoader.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"


FItemDataRow UWizardGameInstance::GetItem(FName RowName)
{
	return FLootPicker::GetItem(RowName);
}

void UWizardGameInstance::Init()
{
	Super::Init();
	if (ItemDataTable)
	{
		FLootPicker::Initialize(ItemDataTable);
	}
	InitDelay();
}

void UWizardGameInstance::InitDelay()
{
	IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld());
	
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Failed to get Online Subsystem!"), *FString(__FUNCTION__));
		return;
	}

	if (IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld()))
	{
		SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UWizardGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UWizardGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UWizardGameInstance::OnJoinSessionCompleted);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UWizardGameInstance::OnDestroySessionComplete);
		}
	}
}

void UWizardGameInstance::Host_LanSession(FString SessionName)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: SessionInterface is not valid!"), *FString(__FUNCTION__));
		return;
	}
	
	ActiveSessionName = NAME_GameSession;

	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = true;
	SessionSettings->NumPublicConnections = 3;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = false;
	SessionSettings->bUsesPresence = false;
	SessionSettings->bUseLobbiesIfAvailable = false;
	
	SessionSettings->Set(FName(TEXT("SESSION_NAME")), SessionName, EOnlineDataAdvertisementType::ViaOnlineService);

	SessionInterface->CreateSession(0, ActiveSessionName, *SessionSettings);
}

void UWizardGameInstance::Find_LanSessions()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: SessionInterface is not valid!"), *FString(__FUNCTION__));
		return;
	}
	
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = 10;
	SessionSearch->PingBucketSize = 100;
	
	//SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, false, EOnlineComparisonOp::Equals);
	
	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UWizardGameInstance::Join_LanSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: SessionInterface or SessionSearch is not valid!"), *FString(__FUNCTION__));
		return;
	}
	
	if (!SessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Invalid SessionIndex!"), *FString(__FUNCTION__));
		return;
	}
	
	const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[SessionIndex];
	
	if (!Result.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Selected session result is not valid!"), *FString(__FUNCTION__));
		return;
	}
	
	int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
	int32 OpenSlots = Result.Session.NumOpenPublicConnections;

	if (MaxPlayers <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Session has invalid MaxPlayers (%d)."), *FString(__FUNCTION__), MaxPlayers);
		return;
	}

	if (OpenSlots <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Session is full (OpenSlots=%d, MaxPlayers=%d)."), *FString(__FUNCTION__), OpenSlots, MaxPlayers);
		return;
	}
	
	ULocalPlayer* Player = GetFirstGamePlayer();
	int32 LocalUserNum = Player ? Player->GetControllerId() : 0;
	SessionInterface->JoinSession(LocalUserNum, NAME_GameSession, Result);
}

TArray<FSessionProps> UWizardGameInstance::GetLanSessions()
{
	TArray<FSessionProps> OutResults;
	if (!SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: SessionSearch is not valid!"), *FString(__FUNCTION__));
		return OutResults;
	}
	
	if (LanSessionResults.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No LAN Sessions found!"), *FString(__FUNCTION__));
		return OutResults;
	}
	
	OutResults.Reserve(LanSessionResults.Num());
	
	for (int32 Index = 0; Index < LanSessionResults.Num(); ++Index)
	{
		const FOnlineSessionSearchResult& Result = LanSessionResults[Index];
		
		FString DisplayName;
		if (!Result.Session.SessionSettings.Get(FName(TEXT("SESSION_NAME")), DisplayName))
		{
			DisplayName = Result.Session.OwningUserName;
		}
		
		int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
		int32 CurrentPlayers = MaxPlayers - Result.Session.NumOpenPublicConnections;
		
		if (CurrentPlayers < 0)
		{
			CurrentPlayers = 0;
		}
		
		FSessionProps Props(Index, DisplayName, CurrentPlayers, MaxPlayers);
		OutResults.Add(Props);
	}
	return OutResults;
}

void UWizardGameInstance::OpenMainMenuMap(const FString& MainMenuMap)
{
	UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("%s: World is null!"), *FString(__FUNCTION__));
        return;
    }

	//Needed?
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: No SessionInterface, opening main menu locally."), *FString(__FUNCTION__));
		UGameplayStatics::OpenLevel(this, FName(*MainMenuMap));
		return;
	}

	ENetMode NetMode = World->GetNetMode();
	if (NetMode == NM_ListenServer || NetMode == NM_DedicatedServer)
	{
		PendingMainMenuMap = MainMenuMap;

		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			APlayerController* PC = It->Get();
			if (!PC)
			{
				continue;
			}
			
			if (PC->IsLocalController())
			{
				continue;
			}
			
			PC->ClientTravel(*MainMenuMap, TRAVEL_Absolute);
		}
		
		World->GetTimerManager().SetTimer(DestroySessionTimerHandle, this, &UWizardGameInstance::TriggerDestroySession, DestroySessionDelaySeconds, false, false);
	}
}

void UWizardGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (SessionInterface.IsValid())
		{
			ULocalPlayer* Player = GetFirstGamePlayer();
			if (Player)
			{
				TSharedPtr<const FUniqueNetId> UserID = Player->GetPreferredUniqueNetId().GetUniqueNetId();
				SessionInterface->RegisterPlayer(SessionName, *UserID, false);
			}
		}
		if (GetWorld())
		{
			GetWorld()->ServerTravel(TEXT("/Game/Start?listen"));
		}
		UE_LOG(LogTemp, Warning, TEXT("%s: Session %s created successfully."), *FString(__FUNCTION__), *SessionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Failed to create session %s."), *FString(__FUNCTION__), *SessionName.ToString());
	}
}

void UWizardGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	LanSessionResults.Empty();
	
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		for (auto& Result : SessionSearch->SearchResults)
		{
			LanSessionResults.Add(Result);
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%s: Found %d LAN Sessions."), *FString(__FUNCTION__), LanSessionResults.Num());
}

void UWizardGameInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("%s: SessionInterface is not valid!"), *FString(__FUNCTION__));
		return;
	}
	
	if (SessionInterface.IsValid())
	{
		ULocalPlayer* Player = GetFirstGamePlayer();
		if (Player)
		{
			TSharedPtr<const FUniqueNetId> UserID = Player->GetPreferredUniqueNetId().GetUniqueNetId();
			SessionInterface->RegisterPlayer(SessionName, *UserID, false);
		}
	}

	if (FString ConnectString; SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		
		if (PC)
		{
			PC->ClientTravel(ConnectString, TRAVEL_Absolute);
		}
	}
}

void UWizardGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnDestroySessionComplete: %s Success=%d"), *SessionName.ToString(), bWasSuccessful);
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DestroySessionTimerHandle);
	}
	
	if (SessionName == ActiveSessionName)
	{
		ActiveSessionName = NAME_GameSession;
	}

	if (!bWasSuccessful)
	{
		UE_LOG(LogTemp, Error, TEXT("OnDestroySessionComplete: Failed to destroy session."));
		PendingMainMenuMap.Empty();
		return;
	}

	if (!PendingMainMenuMap.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*PendingMainMenuMap));
		PendingMainMenuMap.Empty();
	}
}

void UWizardGameInstance::TriggerDestroySession()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->DestroySession(ActiveSessionName);
	}
	else
	{
		if (!PendingMainMenuMap.IsEmpty())
		{
			UGameplayStatics::OpenLevel(this, FName(*PendingMainMenuMap));
			PendingMainMenuMap.Empty();
		}
	}
}


