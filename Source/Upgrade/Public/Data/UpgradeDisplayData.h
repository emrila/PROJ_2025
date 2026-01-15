// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Data/ModifierData.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> Mesh = nullptr/* = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")))*/;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName RowName = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentUpgradeLevel = 0;

	UPROPERTY(BlueprintReadWrite)
	FName TargetPlayerName = NAME_Name;

	bool operator==(const FUpgradeDisplayData& UpgradeData) const
	{
		return RowName == UpgradeData.RowName && Title.EqualTo(UpgradeData.Title) && Description.EqualTo(UpgradeData.Description) && Icon == UpgradeData.Icon;
	};

	bool operator!=(const FUpgradeDisplayData& UpgradeData) const
	{
		return !(*this == UpgradeData);
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("RowName: %s, Title: %s, Description: %s"), *RowName.ToString(), *Title.ToString(), *Description.ToString());
	}
};


USTRUCT(BlueprintType)
struct FUpgradeStatsDisplay
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta =(TitleProperty="RowName"))
	TArray<FUpgradeDisplayData> Items;
};


USTRUCT(BlueprintType)
struct UPGRADE_API FAttributeUpgradeData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "/Script/Upgrade.EUpgradeFlag"))
	int32 UpgradeFlags = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUpgradeDisplayData UpgradeDisplayData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLinear = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip = "-1 = Infinite upgrades", editcondition = "bIsLinear", editconditionHides))
	int32 MaxNumberOfUpgrades = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,	meta=(ToolTip = "-1 = Infinite downgrades", editcondition = "bIsLinear", editconditionHides))
	int32 MaxNumberOfDowngrades = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,	meta=(ToolTip = "Used as: value  * Multiplier", editcondition = "bIsLinear", editconditionHides))
	FModiferData ModifierData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta =(editcondition = "!bIsLinear", editconditionHides))
	bool bCapAtMaxValue = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta =(TitleProperty="Multiplier", editcondition = "!bIsLinear", editconditionHides))
	TArray<FModiferData> UpgradeModifiers =
	{
		FModiferData()
	};

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere)
	FName DevComment = NAME_None;
#endif

	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override
	{
		UpgradeDisplayData.RowName = InRowName;
	}

	FModiferData GetModifier(const int32 Level) const
	{
		return bIsLinear ? ModifierData : UpgradeModifiers[Level];
	}

	bool CanUpgrade(const int32 CurrentLevel) const
	{
		if (bIsLinear)
		{
			return MaxNumberOfUpgrades > CurrentLevel || MaxNumberOfUpgrades == -1;
		}

		const bool bHasNext = UpgradeModifiers.IsValidIndex(CurrentLevel + 1);
		return bCapAtMaxValue
			       ? bHasNext
			       : bHasNext || UpgradeModifiers.IsValidIndex(CurrentLevel);
	}

	bool CanDowngrade(const int32 CurrentLevel) const
	{
		return bIsLinear
			       ? MaxNumberOfDowngrades < CurrentLevel || MaxNumberOfDowngrades == -1
			       : UpgradeModifiers.IsValidIndex(CurrentLevel - 1);
	}

	bool IsMatch(UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/Upgrade.EUpgradeFlag")) int32 Bitmask) const
	{
		return (Bitmask & UpgradeFlags) == Bitmask;
	}
};
