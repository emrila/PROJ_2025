// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "TeamModifierData.generated.h"

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

	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;
};
