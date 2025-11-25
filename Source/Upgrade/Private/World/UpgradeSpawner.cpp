// Furkan approves of this


#include "World/UpgradeSpawner.h"

#include "Interactor.h"
#include "Core/UpgradeComponent.h"
#include "Dev/UpgradeLog.h"
#include "Net/UnrealNetwork.h"
#include "Util/UpgradeFunctionLibrary.h"
#include "World/UpgradeAlternative.h"

AUpgradeSpawner::AUpgradeSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;
	PrimaryActorTick.TickInterval = 1.f;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	
	SpawnSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SpawnSplineComponent"));
	SpawnSplineComponent->SetupAttachment(RootComponent);	
	
	bReplicates = true;

}

void AUpgradeSpawner::OnUpgradeCompleted()
{
	UPGRADE_DISPLAY(TEXT("🎶%hs: Upgrade completed."), __FUNCTION__);
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
		AUpgradeAlternative* UpgradeAlternative = UpgradeAlternativePair.Alternative;
		if (!UpgradeAlternative)
		{
			UPGRADE_WARNING(TEXT("%hs: UpgradeAlternativePair.Alternative is null!"), __FUNCTION__);
			continue;
		}
		UpgradeAlternative->SetUpgradeDisplayData(UpgradeAlternativePair.UpgradeData);

		if (!UpgradeAlternative->OnPostUpgrade.IsAlreadyBound(this, &AUpgradeSpawner::LockUpgradeAlternatives))
		{
			UpgradeAlternative->OnPostUpgrade.AddDynamic(this, &AUpgradeSpawner::LockUpgradeAlternatives);
		}

		UPGRADE_DISPLAY(TEXT("%hs: Assigned upgrade to spawned alternative."), __FUNCTION__);
	}
}

void AUpgradeSpawner::Server_Spawn_Implementation()
{
	if (!SpawnSplineComponent || NumberOfSpawnAlternatives <= 0 || !UpgradeAlternativePairs.IsEmpty())
	{
		return;
	}

	UClass* AlternativeClass = UpgradeAlternativeClass.LoadSynchronous();
	if (!AlternativeClass)
	{
		return;
	}
	UUpgradeComponent* UpgradeComp = UUpgradeFunctionLibrary::GetLocalUpgradeComponent(this);
	const TArray<FUpgradeDisplayData> LocalUpgradeDataArray = UpgradeComp ? UpgradeComp->GetRandomUpgrades(NumberOfSpawnAlternatives) : TArray<FUpgradeDisplayData>();	

	const float SplineLength = SpawnSplineComponent->GetSplineLength();
   	const float SegmentLength = SplineLength / (NumberOfSpawnAlternatives + 1);
	
	TArray<FUpgradeAlternativePair> LocalUpgradeAlternativePairs; //Waiting to trigger OnRep on clients after all alternatives are spawned
	
	for (int32 i = 0; i < LocalUpgradeDataArray.Num(); ++i)
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
		
		if (!LocalUpgradeDataArray.IsValidIndex(i)) //Shouldn't be needed... But just in case
		{
			UPGRADE_ERROR(TEXT("%hs: RandomIndex %d is invalid!? Actual size: %d"), __FUNCTION__, i, LocalUpgradeDataArray.Num());
			break;
		}
		LocalUpgradeAlternativePairs.Emplace(SpawnedAlternative, LocalUpgradeDataArray[i]); //Waiting to trigger OnRep on clients after all alternatives are spawned
		SpawnedAlternative->Index = i;		
		UPGRADE_DISPLAY(TEXT("%hs: Spawned alternative index: %d"), __FUNCTION__, i);
	}
	UpgradeAlternativePairs = LocalUpgradeAlternativePairs;
}

#if WITH_EDITOR
void AUpgradeSpawner::PostLoad()
{
	Super::PostLoad();
    
	if (UpgradeDataArray.IsEmpty() && !PlayerUpgradeDisplayEntry.UpgradeDataArray.IsEmpty())
	{
		UpgradeDataArray = PlayerUpgradeDisplayEntry.UpgradeDataArray;
		UPGRADE_DISPLAY(TEXT("%hs: Migrated data to UpgradeDataArray"), __FUNCTION__);
		
		// MarkPackageDirty();
	}
}
#endif

void AUpgradeSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (bSpawnOnBeginPlay) 
	{
		TriggerSpawn();
	}	
}

void AUpgradeSpawner::LockUpgradeAlternatives()
{
	for (const FUpgradeAlternativePair& UpgradeAlternativePair : UpgradeAlternativePairs)
	{
		if (UpgradeAlternativePair.Alternative)
		{
			UpgradeAlternativePair.Alternative->SetLocked(true);	 
			UPGRADE_DISPLAY(TEXT("%hs: Locked alternative. Is Selected : %s"), __FUNCTION__, UpgradeAlternativePair.Alternative->bSelected ? TEXT("true") : TEXT("false"));
		}		
	}
}

void AUpgradeSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUpgradeSpawner, PlayerUpgradeDisplayEntry);
	DOREPLIFETIME(AUpgradeSpawner, UpgradeAlternativePairs);
	DOREPLIFETIME(AUpgradeSpawner, bSpawnOnBeginPlay);
	DOREPLIFETIME(AUpgradeSpawner, NumberOfSpawnAlternatives);
	DOREPLIFETIME(AUpgradeSpawner, UpgradeDataArray);
	DOREPLIFETIME(AUpgradeSpawner, TotalUpgradeNeededForCompletion);
}

void AUpgradeSpawner::OnRep_UpgradeAlternativePairs()
{
	UPGRADE_DISPLAY(TEXT("%hs: called."), __FUNCTION__);
	ShowAllUpgradeAlternatives(UpgradeAlternativePairs);
}

void AUpgradeSpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		int32 Completed = 0;
		for (const FUpgradeAlternativePair& UpgradeAlternativePair : UpgradeAlternativePairs)
		{
			if (UpgradeAlternativePair.Alternative && UpgradeAlternativePair.Alternative->bSelected)
			{
				Completed++;
			}
		}
		if (Completed >= TotalUpgradeNeededForCompletion)
		{
			UPGRADE_DISPLAY(TEXT("%hs: All upgrades selected, broadcasting event."), __FUNCTION__);
			OnCompletedAllUpgrades.Broadcast();
			SetActorTickEnabled(false);
		}
	}
}

void AUpgradeSpawner::OnInteract_Implementation(UObject* Interactor)
{
	if (!Execute_CanInteract(this))
	{
		return;
	}
	TriggerSpawn();
	if (Interactor && Interactor->Implements<IInteractor::UClassType>())
	{
		IInteractor::Execute_OnFinishedInteraction(Interactor, this);
	}
		
}

bool AUpgradeSpawner::CanInteract_Implementation()
{
	return UpgradeAlternativePairs.IsEmpty();
}
