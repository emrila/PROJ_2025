// Furkan approves of this


#include "World/UpgradeSpawner.h"

#include "World/UpgradeAlternative.h"
#include "Dev/UpgradeLog.h"
#include "Net/UnrealNetwork.h"

AUpgradeSpawner::AUpgradeSpawner()
{
	SpawnSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SpawnSplineComponent"));
	bReplicates = true;
}

void AUpgradeSpawner::TriggerSpawn()
{
	if (HasAuthority())
	{
		Server_Spawn();
		UPGRADE_DISPLAY(TEXT("%hs: Server_Spawn completed."), __FUNCTION__);
	}
	ShowAllUpgradeAlternatives(UpgradeAlternativePairs);
}

void AUpgradeSpawner::ShowAllUpgradeAlternatives(const TArray<FUpgradeAlternativePair> InAssignableUpgrades)
{
	for (const FUpgradeAlternativePair& UpgradeAlternativePair : InAssignableUpgrades)
	{
		if (UpgradeAlternativePair.Alternative)
		{
			UpgradeAlternativePair.Alternative->SetUpgradeDisplayData(UpgradeAlternativePair.UpgradeData);
			UpgradeAlternativePair.Alternative->OnUpgrade.AddDynamic(this, &AUpgradeSpawner::OnUpgradeSelected);
			UPGRADE_DISPLAY(TEXT("%hs: Assigned upgrade to spawned alternative."), __FUNCTION__);
		}
		else
		{
			UPGRADE_WARNING(TEXT("%hs: UpgradeAlternativePair.Alternative is null!"), __FUNCTION__);
		}
	}
}

void AUpgradeSpawner::Server_Spawn_Implementation()
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

	TArray<FUpgradeDisplayData> UpgradeDataArray = PlayerUpgradeDisplayEntry.UpgradeDataArray; // Local copy or reference?
	TArray<FUpgradeAlternativePair> LocalUpgradeAlternativePairs;

	for (int32 i = 1; i <= NumberOfSpawnAlternatives; ++i)
	{
		const float Distance = SegmentLength * i;
		const FVector Location = SpawnSplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		const FRotator Rotation = SpawnSplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AUpgradeAlternative* SpawnedAlternative = GetWorld()->SpawnActor<AUpgradeAlternative>(AlternativeClass, Location, Rotation, SpawnParams);
		if (!SpawnedAlternative)
		{
			continue; // Or break?
		}
		SpawnedAlternative->AttachToComponent(SpawnSplineComponent, FAttachmentTransformRules::KeepWorldTransform);

		const int32 RandomIndex = FMath::RandRange(0, UpgradeDataArray.Num() - 1);
		if (!UpgradeDataArray.IsValidIndex(RandomIndex))  //Shouldn't be needed... But just in case
		{
			UPGRADE_ERROR(TEXT("%hs: RandomIndex %d is invalid!? Actual size: %d"), __FUNCTION__, RandomIndex, UpgradeDataArray.Num());
			break;
		}

		LocalUpgradeAlternativePairs.Emplace(SpawnedAlternative, UpgradeDataArray[RandomIndex]); //Waiting to trigger OnRep on clients after all alternatives are spawned
		UpgradeDataArray.RemoveAt(RandomIndex); // To avoid duplicates
		UPGRADE_DISPLAY(TEXT("%hs: Spawned alternative index: %d"), __FUNCTION__, RandomIndex);
	}
	UpgradeAlternativePairs = LocalUpgradeAlternativePairs;
}

bool AUpgradeSpawner::Server_Spawn_Validate()
{
	return true;
}



void AUpgradeSpawner::BeginPlay()
{
	Super::BeginPlay();
	TriggerSpawn();
}

void AUpgradeSpawner::OnUpgradeSelected()
{
	UPGRADE_DISPLAY(TEXT("%hs: An upgrade alternative was selected."), __FUNCTION__);
}

void AUpgradeSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUpgradeSpawner, PlayerUpgradeDisplayEntry);
	DOREPLIFETIME(AUpgradeSpawner, UpgradeAlternativePairs);
}



void AUpgradeSpawner::OnRep_UpgradeAlternativePairs()
{
	UPGRADE_DISPLAY(TEXT("%hs: called."), __FUNCTION__);
	ShowAllUpgradeAlternatives(UpgradeAlternativePairs);
}
