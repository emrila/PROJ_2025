// Furkan approves of this


#include "RoomSpawnPoint.h"

#include "Components/ArrowComponent.h"


ARoomSpawnPoint::ARoomSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	RootComponent = ArrowComponent;
}
