// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomManagerBase.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/Actor.h"
#include "RoomLoader.generated.h"

UCLASS()
class PROJ_2025_API ARoomLoader : public AActor
{
	GENERATED_BODY()
	
public:	

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Rooms")
	URoomData* CurrentRoom = nullptr;


	UPROPERTY()
	FName CurrentLoadedLevelName;
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Rooms")
	void LoadNextRoom(URoomData* NextRoomData);
	
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
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float IncrementPerScale = 0.2f;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	int OneTwoThreeScale = 0;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	int ClearedRooms = 0;
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(Replicated)
	float DungeonScaling = 1.f;
	
	UPROPERTY(ReplicatedUsing=OnRep_PastSevenRooms)
	TArray<ERoomType> PastSevenRooms;

	UFUNCTION()
	void OnRep_PastSevenRooms();
	
	UPROPERTY(Replicated)
	URoomData* PendingNextRoomData = nullptr;

	FTimerHandle UnloadCheckHandle;
	

	UFUNCTION()
	void OnNextLevelLoaded();
	
	UFUNCTION()
	void OnPreviousLevelUnloaded();

};
