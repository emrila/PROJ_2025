// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShadowStrikeRibbon.generated.h"

UCLASS()
class PROJ_2025_API AShadowStrikeRibbon : public AActor
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SpawnRibbon(FVector StartLocation, FVector EndLocation, float Duration);
};
