// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RoomManagerBase.h"
#include "WizardGameInstance.generated.h"


/**
 * 
 */
UCLASS()
class PROJ_2025_API UWizardGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Rooms")
	TArray<URoomData*> GetAllRoomData() const;

	UPROPERTY(BlueprintReadOnly, Category = "Rooms")
	class ARoomLoader* RoomLoader = nullptr;

};
