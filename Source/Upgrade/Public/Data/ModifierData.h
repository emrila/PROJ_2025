// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "ModifierData.generated.h"

USTRUCT(BlueprintType)
struct FModiferData
{
	GENERATED_BODY()

	FModiferData() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip = "Should the initial value be added to or multiplied by the Multiplier"))
	bool bUseAddition = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ToolTip = "Used as: value * Multiplier OR value + Multiplier"))
	float Multiplier = 0.1f;

	bool bRemoveModifier = false;

	template <typename T>
	T ApplyModifier(const T& BaseValue) const
	{
		return bUseAddition ? BaseValue + Multiplier : BaseValue * Multiplier;
	}
};
