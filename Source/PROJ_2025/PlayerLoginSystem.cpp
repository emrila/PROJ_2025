// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerLoginSystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"



void UPlayerLoginSystem::StartLoginProcess()
{
	if (const IOnlineSubsystem* OnlineSubsystem = Online::GetSubsystem(GetWorld()))
	{
		if (const IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
		{
			if (const TSharedPtr<const FUniqueNetId> UserId = Identity->GetUniquePlayerId(0))
			{
				FString SteamID = UserId->ToString();
				PlayerProfile.SteamID = SteamID;
				UE_LOG(LogTemp, Log, TEXT("Steam ID: %s"), *SteamID);
				OnUsernameRequired.Broadcast(); // ta bort sen
				//SendCheckPlayerRequest(SteamID);
			}
		}
	}
}

void UPlayerLoginSystem::SendCheckPlayerRequest(const FString& SteamID)
{
	auto& Http = FHttpModule::Get();
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = Http.CreateRequest();
	FString URL = "https://game-backend-z56e.onrender.com/players/id?steam_id=" + SteamID;
	
	Request->SetURL(URL);
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "application/json");
	
	Request->OnProcessRequestComplete().BindUObject(this, &UPlayerLoginSystem::OnCheckPlayerResponse);
	Request->ProcessRequest();
}

void UPlayerLoginSystem::OnCheckPlayerResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get player info from server"));
		return;
	}

	if (Response->GetResponseCode() == 200) //player found
	{
		FString ResponseStr = Response->GetContentAsString();
		UE_LOG(LogTemp, Log, TEXT("Player exists response: %s"), *ResponseStr);

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			PlayerProfile.Username = JsonObject->GetStringField(TEXT("username"));
			PlayerProfile.Level = JsonObject->GetIntegerField(TEXT("level"));
			PlayerProfile.Xp = JsonObject->GetIntegerField(TEXT("xp"));
			PlayerProfile.Gold = JsonObject->GetIntegerField(TEXT("gold"));

			UE_LOG(LogTemp, Log, TEXT("Player username: %s"), *PlayerProfile.Username);
			UE_LOG(LogTemp, Log, TEXT("Player level: %d"), PlayerProfile.Level);
			UE_LOG(LogTemp, Log, TEXT("Player XP: %d"), PlayerProfile.Xp);
			UE_LOG(LogTemp, Log, TEXT("Player gold: %d"), PlayerProfile.Gold);

			UE_LOG(LogTemp, Log, TEXT("Player logged in successfully"));
			
			OnLoginSuccess.Broadcast();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("some JSON error"));
		}
	}
	else if (Response->GetResponseCode() == 404) //player not found
	{
		UE_LOG(LogTemp, Log, TEXT("Player not found, need to create new player"));
		OnUsernameRequired.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unexpected response code: %d"), Response->GetResponseCode());
	}
}

void UPlayerLoginSystem::SendCreatePlayerRequest(const FString& Username)
{
	if (PlayerProfile.SteamID.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No Steam ID found"));
		return;
	}
	PlayerProfile.Username = Username;
	PlayerProfile.Xp = 0;
	PlayerProfile.Level = 0;
	PlayerProfile.Gold = 0;

	OnLoginSuccess.Broadcast(); // ta bort sen
	/*
	const TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
	JsonObject->SetStringField(TEXT("steam_id"), PlayerProfile.SteamID);
	JsonObject->SetStringField(TEXT("username"), PlayerProfile.Username);

	FString RequestBody;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL("https://game-backend-z56e.onrender.com/players");
	Request->SetVerb("POST");
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("x-api-key"), TEXT("kFdp2Nk$*`/QUrE")); // DET HÄR ÄR NOG INTE SÄKERT
	Request->SetContentAsString(RequestBody);
	Request->OnProcessRequestComplete().BindUObject(this, &UPlayerLoginSystem::OnCreatePlayerResponse);
	Request->ProcessRequest();
	*/
}

void UPlayerLoginSystem::OnCreatePlayerResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) const
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create player."));
		return;
	}

	if (Response->GetResponseCode() == 201) //player created
	{
		UE_LOG(LogTemp, Log, TEXT("Player successfully created."));
		OnLoginSuccess.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Player creation failed"));
	}
}