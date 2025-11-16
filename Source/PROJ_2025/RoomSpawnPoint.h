// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomSpawnPoint.generated.h"

UCLASS()
class PROJ_2025_API ARoomSpawnPoint : public AActor
{
	GENERATED_BODY()
public:
	ARoomSpawnPoint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UArrowComponent* ArrowComponent;

};
