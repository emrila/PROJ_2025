// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "UpgradeDisplayData.generated.h"

USTRUCT(BlueprintType)
struct FUpgradeDisplayData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title = FText::FromString("Upgrade Title");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description = FText::FromString("Upgrade Description");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/VREditor/Devices/Vive/UE4_Logo.UE4_Logo")));

	FName RowName = NAME_None;
};

USTRUCT(BlueprintType)
struct FPlayerUpgradeDisplayEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUpgradeDisplayData> UpgradeDataArray = {};
};

USTRUCT(BlueprintType)
struct UPGRADE_API FAttributeUpgradeData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override
	{
		UpgradeDisplayData.RowName = InRowName;
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip = "-1 = Infinite upgrades"))
	FUpgradeDisplayData UpgradeDisplayData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip = "-1 = Infinite upgrades"))
	int32 MaxNumberOfUpgrades = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip = "Used as: value += InitialValue * Multiplier"))
	float Multiplier = 0.1f; //percental increase per upgrade level
};
