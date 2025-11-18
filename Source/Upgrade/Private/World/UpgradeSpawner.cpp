// Furkan approves of this


#include "World/UpgradeSpawner.h"

#include "AdvancedSessionsLibrary.h"
#include "Core/UpgradeComponent.h"
#include "Core/UpgradeSubsystem.h"
#include "Dev/UpgradeLog.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "World/UpgradeAlternative.h"

AUpgradeSpawner::AUpgradeSpawner()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
	SpawnSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SpawnSplineComponent"));
	SpawnSplineComponent->SetupAttachment(RootComponent);
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
		if (!UpgradeAlternativePair.Alternative)
		{
			UPGRADE_WARNING(TEXT("%hs: UpgradeAlternativePair.Alternative is null!"), __FUNCTION__);
			continue;
		}
		UpgradeAlternativePair.Alternative->SetUpgradeDisplayData(UpgradeAlternativePair.UpgradeData);
		if (UpgradeAlternativePair.Alternative->OnUpgrade.IsAlreadyBound( this, &AUpgradeSpawner::OnUpgradeSelected))
		{
			continue;
		}
		UpgradeAlternativePair.Alternative->OnUpgrade.AddDynamic(this, &AUpgradeSpawner::OnUpgradeSelected);
		UpgradeAlternativePair.Alternative->OnPreUpgrade.AddDynamic(this, &AUpgradeSpawner::LockUpgradeAlternatives);
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
	auto GetPlayerPawn = [](const AController* Controller) -> APawn*
	{
		if (!Controller)
		{
			UPGRADE_ERROR( TEXT("%hs: Controller is null!"), __FUNCTION__);
			return nullptr;
		}
		
		if (!Controller->IsA(APlayerController::StaticClass()))
		{
			UPGRADE_ERROR( TEXT("%hs: Controller is not a PlayerController!"), __FUNCTION__);
			return nullptr;
		}
		
		return Controller->GetPawn();
	};
	auto GetUpgradeComponent = [](const APawn* Pawn) -> UUpgradeComponent*
	{
		if (!Pawn)
		{
			return nullptr;
		}
		if (!Pawn->IsA(ACharacter::StaticClass()))
		{
			UPGRADE_ERROR( TEXT("%hs: Pawn is not a Character!"), __FUNCTION__);
			return nullptr;
		}
		UActorComponent* ComponentByClass = Pawn->GetComponentByClass(UUpgradeComponent::StaticClass());

		return ComponentByClass ? Cast<UUpgradeComponent>(ComponentByClass) : nullptr;
	};	
	
	UPGRADE_DISPLAY(TEXT("%hs: An upgrade alternative was selected."), __FUNCTION__);
	const int32 PlayerID = SelectedUpgrade.TargetPlayers.IsEmpty() ? -1 : SelectedUpgrade.TargetPlayers[0];
		
	TArray<UUpgradeComponent*> Targets;
	Targets.SetNum(SelectedUpgrade.TargetPlayers.Num());
	
	int32 Index = 0;
	TArray<TObjectPtr<APlayerState>> PlayerStates = GetWorld()->GetGameState()->PlayerArray;
	for (const TObjectPtr<APlayerState>& PlayerState : PlayerStates)
	{				
		if (!SelectedUpgrade.TargetPlayers.Contains(PlayerState->GetPlayerId()))
		{
			//UPGRADE_WARNING( TEXT("%hs: PlayerID %d not found in upgrade target players."), __FUNCTION__, PlayerState->GetPlayerId());
			continue;
		}
		Index = PlayerStates.IndexOfByKey(PlayerState);
		UPGRADE_DISPLAY(TEXT("%hs: Found Player at index %d"), __FUNCTION__, Index);
		const APawn* Pawn = PlayerState->GetPawn();			
		if (!Pawn)
		{
			UPGRADE_WARNING( TEXT("%hs: PlayerState %s has no pawn!"), __FUNCTION__, *PlayerState->GetName());
			continue;
		}
		UPGRADE_DISPLAY(TEXT("%hs: ID in upgrade: %d ---- ID in PlayerState: %d "), __FUNCTION__, PlayerID, PlayerState->GetPlayerId());
		
		UUpgradeComponent* UpgradeComponent = GetUpgradeComponent(Pawn);
		if (!UpgradeComponent)
		{
			UPGRADE_WARNING(TEXT("%hs: Could not find upgrade component!"), __FUNCTION__);
			continue;
		}
		Targets.Add(UpgradeComponent);
	}
	
	if (APawn* TargetPlayer = UGameplayStatics::GetPlayerPawn(this, Index))
	{
		UPGRADE_DISPLAY( TEXT("%hs: TargetPlayer Pawn found: %s"), __FUNCTION__, *TargetPlayer->GetName());
	}
	
	/*APlayerController* FirstLocalPlayerFromController = GetWorld()->GetFirstPlayerController();
	if (!FirstLocalPlayerFromController)
	{
		return;
	}
	APawn* PawnOrSpectator = FirstLocalPlayerFromController->GetPawnOrSpectator();
	if (!PawnOrSpectator)
	{
		return;
	}
	UActorComponent* ComponentByClass = PawnOrSpectator->GetComponentByClass(UUpgradeComponent::StaticClass());
	if (!ComponentByClass)
	{
		return;
	}
	UUpgradeComponent* UpgradeComponent = Cast<UUpgradeComponent>(ComponentByClass);
	if (!UpgradeComponent)
	{
		return;
	}*/
	
	for (const UUpgradeComponent* Target : Targets)
	{
		if (!Target)
		{
			UPGRADE_ERROR(TEXT("%hs: Target UpgradeComponent is null!"), __FUNCTION__);
			continue;
		}
		Target->UpgradeByRow(SelectedUpgrade.RowName);
	}
}

void AUpgradeSpawner::LockUpgradeAlternatives()
{
	for (const FUpgradeAlternativePair& UpgradeAlternativePair : UpgradeAlternativePairs)
	{
		if (UpgradeAlternativePair.Alternative)
		{
			UpgradeAlternativePair.Alternative->bLocked = true;
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
