// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "Engine/GameInstance.h"
#include "RoomManagerBase.h"
#include "WizardGameInstance.generated.h"


/**
 * 
 */
UCLASS()
class PROJ_2025_API UWizardGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	TArray<URoomData*> GetAllRoomData() const;
	URoomData* GetCampRoomData() const;

	bool RollForCampRoom();

	UPROPERTY(BlueprintReadOnly, Category = "Rooms")
	class ARoomLoader* RoomLoader = nullptr;

private:
	float ChanceForCamp = 0.f;

};
