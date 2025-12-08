// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Core/AttributeData.h"
#include "UpgradeDisplayData.generated.h"

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EUpgradeFlag : uint8
{
	None = 0 UMETA(DisplayName = "None", Hidden),
	Solo = 1 << 0 UMETA(DisplayName = "Solo"),
	Team = 1 << 1 UMETA(DisplayName = "Team"),
};

ENUM_CLASS_FLAGS(EUpgradeFlag)

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FName RowName = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	FName TargetName = NAME_None;

	bool operator==(const FUpgradeDisplayData& UpgradeData) const
	{
		return RowName == UpgradeData.RowName && Title.EqualTo(UpgradeData.Title) && Description.EqualTo(UpgradeData.Description) && Icon ==
			UpgradeData.Icon;
	};

	bool operator!=(const FUpgradeDisplayData& UpgradeData) const
	{
		return !(*this == UpgradeData);
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("RowName: %s, Title: %s, Description: %s, TargetName: %s"), *RowName.ToString(), *Title.ToString(), *Description.ToString(), *TargetName.ToString());
	}
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

USTRUCT(BlueprintType)
struct UPGRADE_API FModifierEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RowName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowRandomTimesToApply = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin=1, ToolTip="Minimum number of times to apply the modifier"))
	int32 TimesToApply = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldRemove = false;
};

USTRUCT(BlueprintType)
struct UPGRADE_API FTeamModifierData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title = FText::FromString("Upgrade Title");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description = FText::FromString("Upgrade Description");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMesh = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(editcondition="bUseMesh", editconditionHides))
	TSoftObjectPtr<UStaticMesh> Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(editcondition="!bUseMesh", editconditionHides))
	TSoftObjectPtr<UTexture2D> Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/VREditor/Devices/Vive/UE4_Logo.UE4_Logo")));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(TitleProperty="RowName"))
	TArray<FModifierEntry> Modifiers = {};

	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override
	{
		if (Modifiers.IsEmpty())
		{
			const FModifierEntry Item{InRowName};
			Modifiers.Add(Item);
		}
	}
};
