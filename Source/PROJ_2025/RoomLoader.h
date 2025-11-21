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

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddProgressWidget();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	TSubclassOf<UUserWidget> ProgressWidgetClass;
	
	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void IncrementProgress(const bool CountAsClearedRoom);
	
	UFUNCTION(BlueprintCallable, Category = "Rooms")
	float GetDungeonScaling() const;

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void RegisterNextRoom(URoomData* RoomData);

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	TArray<URoomData*> GetPreviousRooms();
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	float IncrementPerScale = 0.2f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
	int OneTwoThreeScale = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int ClearedRooms = 0;
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	float DungeonScaling = 1.f;
	
	
	
	
	UPROPERTY(Replicated)
	TArray<URoomData*> PastSevenRooms;
	
	UPROPERTY(Replicated)
	URoomData* PendingNextRoomData = nullptr;

	FTimerHandle UnloadCheckHandle;
	
	void CheckLevelUnloaded();

	UFUNCTION()
	void OnNextLevelLoaded();
	
	UFUNCTION()
	void OnPreviousLevelUnloaded();

};
