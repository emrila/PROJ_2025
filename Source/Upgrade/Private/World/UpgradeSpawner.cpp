// Furkan approves of this


#include "World/UpgradeSpawner.h"

#include "Core/UpgradeSubsystem.h"
#include "Dev/UpgradeLog.h"
#include "Net/UnrealNetwork.h"
#include "World/UpgradeAlternative.h"

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
			
			if (!UpgradeAlternativePair.Alternative->OnUpgrade.IsAlreadyBound( this, &AUpgradeSpawner::OnUpgradeSelected))
			{
				UpgradeAlternativePair.Alternative->OnUpgrade.AddDynamic(this, &AUpgradeSpawner::OnUpgradeSelected);
				UPGRADE_DISPLAY(TEXT("%hs: Assigned upgrade to spawned alternative."), __FUNCTION__);
			}
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
		SpawnedAlternative->OnStatusChanged.AddDynamic(this, &AUpgradeSpawner::OnAlternativeStatusChanged);
		LocalUpgradeAlternativePairs.Emplace(SpawnedAlternative, UpgradeDataArray[RandomIndex]); //Waiting to trigger OnRep on clients after all alternatives are spawned
		SpawnedAlternative->Index = LocalUpgradeAlternativePairs.Num() - 1;
		UpgradeDataArray.RemoveAt(RandomIndex); // To avoid duplicates
		UPGRADE_DISPLAY(TEXT("%hs: Spawned alternative index: %d"), __FUNCTION__, RandomIndex);
	}
	UpgradeAlternativePairs = LocalUpgradeAlternativePairs;
}

bool AUpgradeSpawner::Server_Spawn_Validate()
{
	return true;
}

void AUpgradeSpawner::OnAlternativeStatusChanged(EUpgradeSelectionStatus NewStatus, int32 Index)
{
	/*const auto IsSelectedOrHovered = [](const EUpgradeSelectionStatus Status)
	{
		return Status == EUpgradeSelectionStatus::Selected || Status == EUpgradeSelectionStatus::Hovered;
	};
	bool bStopStatusChange = false;
	AUpgradeAlternative* UpgradeAlternativeCaller = nullptr;

	for (int i = 0; i < UpgradeAlternativePairs.Num(); ++i)
	{
		AUpgradeAlternative* UpgradeAlternative = UpgradeAlternativePairs[i].Alternative;

		if (!UpgradeAlternative)
		{
			continue;
		}
		if (i == Index) // Found Caller
		{
			UpgradeAlternativeCaller = UpgradeAlternative;
			UPGRADE_DISPLAY(TEXT("%hs: Found caller at index %d."), __FUNCTION__, Index);
			continue;
		}

		if (IsSelectedOrHovered(NewStatus) && IsSelectedOrHovered(UpgradeAlternative->CurrentSelectionStatus))
		{
			// Another alternative is already selected or hovered, disallow change
			UPGRADE_DISPLAY(TEXT("%hs: Another alternative is already selected or hovered, disallowing change."), __FUNCTION__);
			bStopStatusChange = true;
			break;
		}

		UPGRADE_DISPLAY(TEXT("%hs: Setting alternative at index %d status to NotSelected."), __FUNCTION__, i);
		UpgradeAlternative->SetCurrentSelectionStatus(EUpgradeSelectionStatus::NotSelected);
	}

	if (UpgradeAlternativeCaller)
	{
		UPGRADE_DISPLAY(TEXT("%hs: Setting caller status to %s."), __FUNCTION__, bStopStatusChange ? TEXT("NotSelected") : TEXT("Hovered"));
		UpgradeAlternativeCaller->SetCurrentSelectionStatus(bStopStatusChange ? EUpgradeSelectionStatus::NotSelected : EUpgradeSelectionStatus::Hovered);
	}*/

}

void AUpgradeSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (bSpawnOnBeginPlay) 
	{
		TriggerSpawn();
	}
}

void AUpgradeSpawner::OnUpgradeSelected(FUpgradeDisplayData SelectedUpgrade)
{
	UPGRADE_DISPLAY(TEXT("%hs: An upgrade alternative was selected."), __FUNCTION__);
	if (UUpgradeSubsystem* UpgradeSubsystem = UUpgradeSubsystem::Get(GetWorld()))
	{
		UpgradeSubsystem->UpgradeByRow(SelectedUpgrade.RowName);			
	}
	for (const FUpgradeAlternativePair& UpgradeAlternativePair : UpgradeAlternativePairs)
	{
		if (SelectedUpgrade == UpgradeAlternativePair.UpgradeData)
		{
			if (!UpgradeAlternativePair.Alternative)
			{
				UpgradeAlternativePair.Alternative->bLocked = true;
			}
		}
	} 
}

void AUpgradeSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUpgradeSpawner, PlayerUpgradeDisplayEntry);
	DOREPLIFETIME(AUpgradeSpawner, UpgradeAlternativePairs);
	DOREPLIFETIME(AUpgradeSpawner, bSpawnOnBeginPlay);
}

void AUpgradeSpawner::OnRep_UpgradeAlternativePairs()
{
	UPGRADE_DISPLAY(TEXT("%hs: called."), __FUNCTION__);
	ShowAllUpgradeAlternatives(UpgradeAlternativePairs);
}

