// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "Engine/GameInstance.h"
#include "RoomManagerBase.h"
#include "RoomModifierBase.h"
#include "WizardGameInstance.generated.h"


class UWidget;

USTRUCT(BlueprintType)
struct FSessionProps
{
	GENERATED_BODY()
	
	FSessionProps()
		: SessionName(TEXT(""))
		, IndexNum(0)
		, CurrentNumOfPlayers(0)
		, MaxNumOfPlayers(0)
	{}

	explicit FSessionProps(int32 IndexNum_, FString SessionName_, int32 CurrentNumOfPlayers_, int32 MaxNumOfPlayers_)
		: SessionName(MoveTemp(SessionName_))
		, IndexNum(IndexNum_)
		, CurrentNumOfPlayers(CurrentNumOfPlayers_)
		, MaxNumOfPlayers(MaxNumOfPlayers_)
	{}

	UPROPERTY(BlueprintReadOnly)
	FString SessionName;

	UPROPERTY(BlueprintReadOnly)
	int32 IndexNum;
	
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentNumOfPlayers;
	
	UPROPERTY(BlueprintReadOnly)
	int32 MaxNumOfPlayers;
};

/**
 * 
 */
UCLASS()
class PROJ_2025_API UWizardGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()
public:


	UPROPERTY(BlueprintReadOnly, Category = "Rooms")
	class ARoomLoader* RoomLoader = nullptr;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	TMap<ERoomType, FRoomModifierArray> AvailableModsForRoomType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	UDataTable* ItemDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	URoomData* CampRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	URoomData* ChoiceRoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	TArray<URoomData*> BossRooms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	TArray<URoomData*> StaticNormalMapPool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	TArray<URoomData*> StaticCombatOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	TArray<URoomData*> StaticDevMapPool;
	
	//LAN stuff
	virtual void Init() override;
	
	virtual void InitDelay();
	
	UFUNCTION(BlueprintCallable, Category = "LAN")
	void Host_LanSession(FString SessionName);
	
	UFUNCTION(BlueprintCallable, Category = "LAN")
	void Find_LanSessions();
	
	UFUNCTION(BlueprintCallable, Category = "LAN")
	void Join_LanSession(int32 SessionIndex);
	
	UFUNCTION(BlueprintCallable, Category = "LAN")
	TArray<FSessionProps> GetLanSessions();

	UFUNCTION(BlueprintCallable, Category = "Session")
	void OpenMainMenuMap(const FString& MainMenuMap = TEXT("/Game/MainMenu"));
	
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	void TriggerDestroySession();
	
	IOnlineSessionPtr SessionInterface;
	
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	TArray<FOnlineSessionSearchResult> LanSessionResults;

	FName ActiveSessionName = NAME_GameSession;
	FTimerHandle DestroySessionTimerHandle;
	float DestroySessionDelaySeconds = 1.f;
	FString PendingMainMenuMap;

};
