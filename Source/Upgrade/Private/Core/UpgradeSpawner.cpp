// Furkan approves of this


#include "Core/UpgradeSpawner.h"

#include "Core/UpgradeAlternative.h"
#include "Dev/UpgradeLog.h"

AUpgradeSpawner::AUpgradeSpawner()
{
	SpawnSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SpawnSplineComponent"));
}

void AUpgradeSpawner::OnSpawnAlternatives_Implementation()
{
	if (!SpawnSplineComponent || NumberOfSpawnAlternatives <= 0)
	{
		return;
	}

	UClass* AlternativeClass = UpgradeAlternativeClass.LoadSynchronous();
	if (!AlternativeClass)
	{
		return;
	}
	
	const float SplineLength = SpawnSplineComponent->GetSplineLength();
	const float SegmentLength = SplineLength / (NumberOfSpawnAlternatives + 1);

	for (int32 i = 1; i <= NumberOfSpawnAlternatives; ++i)
	{
		const float Distance = SegmentLength * i;
		const FVector Location = SpawnSplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		const FRotator Rotation = SpawnSplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;		

		if (AUpgradeAlternative* SpawnedAlternative = GetWorld()->SpawnActor<AUpgradeAlternative>(AlternativeClass, Location, Rotation, SpawnParams))
		{
			SpawnedAlternative->AttachToComponent(SpawnSplineComponent, FAttachmentTransformRules::KeepWorldTransform);
			SpawnedAlternative->OnUpgrade.AddDynamic(this, &AUpgradeSpawner::OnUpgradeSelected);
		}
	}
}

void AUpgradeSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	//Should be triggered by some game event
	OnSpawnAlternatives();
}


void AUpgradeSpawner::OnUpgradeSelected()
{
	UPGRADE_DISPLAY(TEXT("%hs: An upgrade alternative was selected."), __FUNCTION__);
}
