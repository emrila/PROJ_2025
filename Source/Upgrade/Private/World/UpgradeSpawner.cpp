// Furkan approves of this

#include "World/UpgradeSpawner.h"

#include "Async/Async_WaitGameplayEvent.h"
#include "Components/ValidationComponent.h"
#include "Components/UpgradeComponent.h"
#include "Util/UpgradeLog.h"
#include "Interfaces/UpgradeDisplayInterface.h"
#include "Net/UnrealNetwork.h"
#include "Util/UpgradeFunctionLibrary.h"
#include "Utility/Tags.h"

AUpgradeSpawner::AUpgradeSpawner()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	
	SpawnSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SpawnSplineComponent"));
	SpawnSplineComponent->SetupAttachment(RootComponent);	
	
	bReplicates = true;

	SpawnClass = TSoftClassPtr<AActor>(FSoftObjectPath(TEXT("/Game/Developer/Emma/BP_UpgradeAlternative.BP_UpgradeAlternative_C")));
}

void AUpgradeSpawner::BeginPlay()
{
	Super::BeginPlay();

	const FGameplayTag ValidationTag = ValidationComponent ? ValidationComponent->LockTag : EVENT_TAG_VALIDATION_LOCK;
	WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(this, ValidationTag, false, false))->
	                   EventReceived.AddDynamic(this, &AUpgradeSpawner::OnValidatedLock);

	if (bSpawnOnBeginPlay)
	{
		TriggerSpawn();
	}
}

void AUpgradeSpawner::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUpgradeSpawner, bSpawnOnBeginPlay);
	DOREPLIFETIME(AUpgradeSpawner, TotalToSpawn);
	DOREPLIFETIME(AUpgradeSpawner, UpgradeDataArray);
}

void AUpgradeSpawner::OnValidatedLock(FGameplayEventData Payload)
{
	if (HasAuthority())
	{
		if (AActor* Target = const_cast<AActor*>(Payload.Target.Get());
			Target && Target->Implements<UUpgradeDisplayInterface>())
		{
			const FInstancedStruct SelectablesData = IUpgradeDisplayInterface::Execute_OnGetUpgradeDisplayData(Target);
			UActorComponent* Component = Payload.Instigator->GetComponentByClass(TSubclassOf<UUpgradeComponent>());
			if (UUpgradeComponent* UpgradeComp = Component ? Cast<UUpgradeComponent>(Component) : nullptr)
			{
				UPGRADE_DISPLAY(TEXT("%hs: Sending selected upgrade data to UpgradeComponent."), __FUNCTION__);
				UpgradeComp->Server_OnUpgradeReceived(SelectablesData);
			}
		}

		if (OnCompletedAllUpgrades.IsBound())
		{
			OnCompletedAllUpgrades.Broadcast();
		}
	}
}

void AUpgradeSpawner::SetAllUpgradesDisplayData()
{
	int32 Index = 0;
	for (const FSelectablesInfo& SelectablesInfo : ValidationComponent->GetAllSelectablesInfo())
	{
		if (UpgradeDataArray.IsValidIndex(Index) && SelectablesInfo.Selectable && SelectablesInfo.Selectable->Implements<UUpgradeDisplayInterface>())
		{
			FInstancedStruct InstancedStruct = FInstancedStruct::Make(UpgradeDataArray[Index++]);
			IUpgradeDisplayInterface::Execute_OnSetUpgradeDisplayData(SelectablesInfo.Selectable, InstancedStruct);
		}
	}
}

void AUpgradeSpawner::Server_Spawn_Implementation()
{
	if (!SpawnSplineComponent || TotalToSpawn < 1 )
	{
		return;
	}

	if (!UpgradeDataArray.IsEmpty())
	{
		UPGRADE_DISPLAY(TEXT("%hs: Clearing existing upgrade alternatives before spawning new ones."), __FUNCTION__);
		Server_ClearAll();
	}

	UClass* AlternativeClass = SpawnClass.LoadSynchronous();

	if (!AlternativeClass)
	{
		return;
	}

	const float SplineLength = SpawnSplineComponent->GetSplineLength();
	const float SegmentLength = SplineLength / (TotalToSpawn + 1);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < TotalToSpawn; ++i)
	{
		const float Distance = SegmentLength * (i + 1);
		const FVector Location = SpawnSplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		const FRotator Rotation = SpawnSplineComponent->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		if (auto* SpawnedAlternative = GetWorld()->SpawnActor<AActor>(AlternativeClass, Location, Rotation, SpawnParams))
		{
			SpawnedAlternative->AttachToComponent(SpawnSplineComponent, FAttachmentTransformRules::KeepWorldTransform);
		}
	}

	UUpgradeComponent* UpgradeComp = UUpgradeFunctionLibrary::GetLocalUpgradeComponent(this);
	UpgradeDataArray = UpgradeComp ? UpgradeComp->GetRandomUpgrades(TotalToSpawn) : TArray<FUpgradeDisplayData>();

}

void AUpgradeSpawner::TriggerSpawn()
{
	if (HasAuthority())
	{
		Server_Spawn();		
		UPGRADE_DISPLAY(TEXT("%hs: Server_Spawn completed."), __FUNCTION__);
		SetAllUpgradesDisplayData();
	}

}

void AUpgradeSpawner::SetTotalUpgradeNeededForCompletion(const int32 InTotalUpgradeNeededForCompletion)
{
	if (GetOwner()->HasAuthority() && ValidationComponent)
	{
		ValidationComponent->Server_SetTotalExpectedSelections(InTotalUpgradeNeededForCompletion);
	}
}

void AUpgradeSpawner::Server_ClearAll_Implementation()
{
	if (!ValidationComponent)
	{
		 UPGRADE_WARNING(TEXT("%hs: ValidationComponent is null -> nothing to clear."), __FUNCTION__);
		return;
	}
	for (const FSelectablesInfo& SelectablesInfo : ValidationComponent->GetAllSelectablesInfo())
	{
		if (SelectablesInfo.Selectable && SelectablesInfo.Selectable->Implements<UUpgradeDisplayInterface>())
		{
			IUpgradeDisplayInterface::Execute_OnClearUpgradeDisplayData(SelectablesInfo.Selectable);
		}
	}
	UpgradeDataArray.Empty();
}

void AUpgradeSpawner::OnRep_UpgradeDataArray()
{
	if (UpgradeDataArray.IsEmpty())
	{
		return;
	}
	SetAllUpgradesDisplayData();
}
