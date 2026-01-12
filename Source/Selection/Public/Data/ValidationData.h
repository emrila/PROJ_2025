// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "ValidationData.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct SELECTION_API FValidationData
{
	GENERATED_USTRUCT_BODY()

	FValidationData() = default;
	FValidationData(const float InDuration) :  Duration(InDuration){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> EffectClass;

};
