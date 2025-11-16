// Furkan approves of this


#include "EnemySpawn.h"

#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"


AEnemySpawn::AEnemySpawn()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA
	SpawnBox = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBox"));
	SpawnBox->SetupAttachment(RootComponent);
	SpawnBox->SetBoxExtent(FVector(100.f, 100.f, 100.f)); 
	SpawnBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBox->bHiddenInGame = true; 
	SpawnBox->SetMobility(EComponentMobility::Movable);

	SpawnBox->ShapeColor = FColor::Red;  
	SpawnBox->SetLineThickness(10.0f);

#endif
}

