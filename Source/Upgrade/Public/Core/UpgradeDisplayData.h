// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "UpgradeDisplayData.generated.h"

USTRUCT(BlueprintType)
struct FUpgradeDisplayData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/VREditor/Devices/Vive/UE4_Logo.UE4_Logo")));

	// TODO: Add distinguisher
};