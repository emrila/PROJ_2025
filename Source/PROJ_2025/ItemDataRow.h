#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDataRow.generated.h"

UENUM(BlueprintType)
enum class ELootTier : uint8
{
	Common     UMETA(DisplayName = "Common"),
	Rare       UMETA(DisplayName = "Rare"),
	Legendary  UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UItemBase> ItemClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSet<ELootTier> SpawnTiers;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* DroppedMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Icon;
	
};