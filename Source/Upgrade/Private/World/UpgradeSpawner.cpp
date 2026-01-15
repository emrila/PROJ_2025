// Furkan approves of this

#include "World/UpgradeSpawner.h"

#include "Async/Async_WaitGameplayEvent.h"
#include "Components/UpgradeComponent.h"
#include "Components/ValidationComponent.h"
#include "Interfaces/UpgradeDisplayInterface.h"
#include "Net/UnrealNetwork.h"
#include "Util/UpgradeFunctionLibrary.h"
#include "Util/UpgradeLog.h"
#include "Utility/GameplayUtilFunctionLibrary.h"

AUpgradeSpawner::AUpgradeSpawner()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	SkeletalMeshComponent->SetIsReplicated(true);

	SpawnSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SpawnSplineComponent"));
	SpawnSplineComponent->SetupAttachment(RootComponent);	
	
	bReplicates = true;

	SpawnClass = TSoftClassPtr<AActor>(FSoftObjectPath(TEXT("/Game/Developer/Emma/BP_UpgradeAlternative.BP_UpgradeAlternative_C")));
}

void AUpgradeSpawner::BeginPlay()
{
	Super::BeginPlay();

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

void AUpgradeSpawner::SetAllUpgradesDisplayData()
{
	int32 Index = 0;
	for (const FSelectablesInfo& SelectablesInfo : ValidationComponent->GetAllSelectablesInfo())
	{
		if (UpgradeDataArray.IsValidIndex(Index) && SelectablesInfo.Selectable && SelectablesInfo.Selectable->Implements<UUpgradeDisplayInterface>())
		{
			const FInstancedStruct InstancedStruct = FInstancedStruct::Make(UpgradeDataArray[Index++]);
			IUpgradeDisplayInterface::Execute_OnSetUpgradeDisplayData(SelectablesInfo.Selectable, InstancedStruct);
			IUpgradeDisplayInterface::Execute_OnProcessUpgradeDisplayData(SelectablesInfo.Selectable);
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

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]
		{
			UPGRADE_DISPLAY(TEXT("%hs: Event.Object.Open"), __FUNCTION__);
			UGameplayUtilFunctionLibrary::SendGameplayEventToActor(this, this, FGameplayTag::RequestGameplayTag("Event.Object.Open"), this);
		}),3.0f, false);
	}
}

void AUpgradeSpawner::SetTotalUpgradeNeededForCompletion(const int32 InTotalUpgradeNeededForCompletion)
{
	if (GetOwner() && GetOwner()->HasAuthority() && ValidationComponent)
	{
		ValidationComponent->Server_SetTotalExpectedSelections(InTotalUpgradeNeededForCompletion);
	}
}

void AUpgradeSpawner::OnValidation_Implementation(FInstancedStruct Data)
{
	Super::OnValidation_Implementation(Data);

	if (!HasAuthority())
	{
		return;
	}

	const FSelectablesInfos* SelectablesInfos = Data.GetPtr<FSelectablesInfos>();
	if (!SelectablesInfos)
	{
		UPGRADE_WARNING(TEXT("%hs: SelectablesInfos is null -> cannot process upgrade selection."), __FUNCTION__);
		return;
	}

	for (const FSelectablesInfo& Item : SelectablesInfos->Items)
	{
		UObject* Selectable = Item.Selectable;
		if (!Selectable|| !Selectable->Implements<UUpgradeDisplayInterface>())
		{
			UPGRADE_WARNING(TEXT("%hs: Selectable is null or does not implement UpgradeDisplayInterface -> cannot process upgrade selection."), __FUNCTION__);
			continue;
		}

		FInstancedStruct SelectablesData = IUpgradeDisplayInterface::Execute_OnGetUpgradeDisplayData(Selectable);
		const FUpgradeDisplayData* SelectedUpgrade = SelectablesData.GetMutablePtr<FUpgradeDisplayData>();
		if (!SelectedUpgrade)
		{
			UPGRADE_WARNING(TEXT("%hs: SelectedUpgrade is null -> cannot process upgrade selection for %s."), __FUNCTION__, *Selectable->GetName());
			continue;
		}

		for (UObject* Selector : Item.Selectors) //At most 3 elements == 3 players selecting same upgrade
		{
			UUpgradeComponent* UpgradeComp = UUpgradeFunctionLibrary::GetUpgradeComponentFromActor(Cast<AActor>(Selector));
			if (!UpgradeComp)
			{
				continue;
			}

			UpgradeComp->OnUpgradeReceived(SelectablesData);

			for (const FUpgradeDisplayData& PlayerUpgrade : UpgradeComp->GetPlayerUpgrades()) //At most 4 elements
			{
				if (PlayerUpgrade.RowName == SelectedUpgrade->RowName)
				{
					IUpgradeDisplayInterface::Execute_OnSetUpgradeDisplayData(Selectable, FInstancedStruct::Make(PlayerUpgrade));
					IUpgradeDisplayInterface::Execute_OnProcessUpgradeDisplayData(Selectable);
					break;
				}
			}
		}
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]
	{
		UPGRADE_DISPLAY(TEXT("%hs: Event.Object.Open"), __FUNCTION__);
		UGameplayUtilFunctionLibrary::SendGameplayEventToActor(this, this, FGameplayTag::RequestGameplayTag("Event.Object.Close"), this);
	}),3.0f, false);

	if (OnCompletedAllUpgrades.IsBound())
	{
		OnCompletedAllUpgrades.Broadcast();
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
