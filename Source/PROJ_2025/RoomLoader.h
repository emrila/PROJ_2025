// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomManagerBase.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Actor.h"
#include "RoomLoader.generated.h"
USTRUCT(BlueprintType)
struct FRoomLoaderState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Progress")
	int OneTwoThreeScaleState = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Progress")
	TArray<ERoomType> PastSevenRooms;
};

UCLASS()
class PROJ_2025_API ARoomLoader : public AActor
{
	GENERATED_BODY()
	
public:	

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(BlueprintReadWrite, Category = "Rooms")
	FRoomInstance CurrentRoom;


	UPROPERTY()
	FName CurrentLoadedLevelName;
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Rooms")
	void LoadNextRoom(const FRoomInstance& NextRoomData);
	
	void AddProgressWidget();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	TSubclassOf<UUserWidget> ProgressWidgetClass;
	
	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void IncrementProgress(const bool CountAsClearedRoom);
	
	UFUNCTION(BlueprintCallable, Category = "Rooms")
	float GetDungeonScaling() const;

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void RegisterNextRoom(URoomData* RoomData);

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	TArray<ERoomType> GetPreviousRooms();
	
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing=OnRep_RoomLoaderState)
	FRoomLoaderState RoomLoaderState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float IncrementPerScale = 0.2f;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int ClearedRooms = 0;
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(Replicated)
	float DungeonScaling = 1.f;
	
	int OneTwoThreeScale = 0;
	
	UFUNCTION()
	void OnRep_RoomLoaderState();
	
	UPROPERTY()
	FRoomInstance PendingNextRoomData;

	FTimerHandle UnloadCheckHandle;
	

	UFUNCTION()
	void OnNextLevelLoaded();
	
	UFUNCTION()
	void OnPreviousLevelUnloaded();

};
