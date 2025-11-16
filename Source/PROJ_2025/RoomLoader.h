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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms")
	int ClearedRooms = 0;

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	void RegisterNextRoom(URoomData* RoomData);

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	TArray<URoomData*> GetPreviousRooms();


protected:
	virtual void BeginPlay() override;
private:
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
