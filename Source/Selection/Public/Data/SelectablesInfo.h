// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "SelectablesInfo.generated.h"

USTRUCT(BlueprintType)
struct FSelectablesInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly)
	UObject* Selectable = nullptr;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalSelectors = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<UObject*> Selectors;

	UPROPERTY(BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "/Script/Selection.ESelectionNotificationFlags"))
	uint8 EvaluationFlags = 0;
};
