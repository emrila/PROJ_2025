// Furkan approves of this


#include "World/UpgradeAlternative.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/UpgradeSubsystem.h"
#include "World/UI/UpgradeAlternativeWidget.h"
#include "Dev/UpgradeLog.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

namespace UpgradeWidget
{
	UUpgradeAlternativeWidget* Get(const UWidgetComponent* WidgetComponent)
	{
		if (!WidgetComponent || !WidgetComponent->GetWidget())
		{
			UPGRADE_DISPLAY(TEXT("%hs: WidgetComponent or its widget is NULL!"), __FUNCTION__);
			return nullptr;
		}
		return Cast<UUpgradeAlternativeWidget>(WidgetComponent->GetWidget());
	}
}

AUpgradeAlternative::AUpgradeAlternative()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	SetNetCullDistanceSquared(0.0);// Disable distance culling
	SetNetUpdateFrequency(10.0f);// Update more frequently
	SetMinNetUpdateFrequency(2.0f);

	UpgradeWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("UpgradeWidgetComponent"));
	UpgradeTriggerComponent = CreateDefaultSubobject<USphereComponent>(TEXT("UpgradeTriggerComponent"));
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	RootComponent = SceneComponent;
	UpgradeTriggerComponent->SetupAttachment(RootComponent);
	UpgradeWidgetComponent->SetupAttachment(UpgradeTriggerComponent);

	constexpr float SphereRadius = 100.0f;
	UpgradeTriggerComponent->SetSphereRadius(SphereRadius);
	UpgradeTriggerComponent->SetRelativeLocation(FVector(0.0f, 0.0f, SphereRadius/2.f));
	
	UpgradeTriggerComponent->OnComponentBeginOverlap.AddDynamic(this, &AUpgradeAlternative::OnComponentBeginOverlap);
	UpgradeTriggerComponent->OnComponentEndOverlap.AddDynamic(this, &AUpgradeAlternative::OnComponentEndOverlap);
}

void AUpgradeAlternative::SetUpgradeDisplayData(const FUpgradeDisplayData& Data)
{
	UpgradeDisplayData = Data;
	ShowUpgradeDisplayData();
}

void AUpgradeAlternative::ShowUpgradeDisplayData() const
{
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(UpgradeWidgetComponent))
	{
		UpgradeWidget->OnSetUpgradeDisplayData(UpgradeDisplayData);
	}
}

void AUpgradeAlternative::NotifyUpgradeSelected() const
{
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(UpgradeWidgetComponent))
	{
		UpgradeWidget->OnUpgradeSelected(bUpgradeSelected);
	}
	if (bUpgradeSelected)
	{
		if (UUpgradeSubsystem* UpgradeSubsystem = UUpgradeSubsystem::Get(GetWorld()))
		{
			UpgradeSubsystem->UpgradeByRow(UpgradeDisplayData.RowName);			
		}
		
		OnUpgrade.Broadcast();
	}
}

bool AUpgradeAlternative::IsTargetPlayer(const AActor* OtherActor) const
{
	if (const APawn* Pawn = Cast<APawn>(OtherActor))
	{
		return Pawn->IsPlayerControlled();
	}
	return false;
}

void AUpgradeAlternative::Server_NotifyUpgradeSelected_Implementation(bool bInUpgradeSelected)
{
	bUpgradeSelected = bInUpgradeSelected;
	NotifyUpgradeSelected();
}

bool AUpgradeAlternative::Server_NotifyUpgradeSelected_Validate(bool bInUpgradeSelected)
{
	return true;
}

void AUpgradeAlternative::SetCurrentSelectionStatus(const EUpgradeSelectionStatus NewStatus)
{
	CurrentSelectionStatus = NewStatus;
	const bool bShowAsUpgradeSelected = NewStatus == EUpgradeSelectionStatus::Selected || NewStatus == EUpgradeSelectionStatus::Hovered;
	Server_NotifyUpgradeSelected_Implementation(bShowAsUpgradeSelected);
	NotifyUpgradeSelected();
}

void AUpgradeAlternative::OnRep_UpgradeSelected()
{
	UPGRADE_DISPLAY(TEXT("%hs: bUpgradeSelected replicated to %s"), __FUNCTION__, bUpgradeSelected ? TEXT("true") : TEXT("false"));
	NotifyUpgradeSelected();
}

void AUpgradeAlternative::OnInteract_Implementation(UObject* Interactor)
{
	Server_NotifyUpgradeSelected(true);
	NotifyUpgradeSelected();
	CurrentSelectionStatus = EUpgradeSelectionStatus::Selected;
	OnStatusChanged.Broadcast(CurrentSelectionStatus, Index);
}

bool AUpgradeAlternative::CanInteract_Implementation(UObject* Interactor)
{
	return CurrentSelectionStatus == EUpgradeSelectionStatus::Hovered;
}

void AUpgradeAlternative::OnComponentBeginOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComp, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex, [[maybe_unused]] bool bFromSweep, [[maybe_unused]] const FHitResult& SweepResult)
{
	if (IsTargetPlayer(OtherActor) && Execute_CanInteract(this, OtherActor))
	{		
		CurrentSelectionStatus = EUpgradeSelectionStatus::Hovered;
		OnStatusChanged.Broadcast(CurrentSelectionStatus, Index);

		/*
		Server_NotifyUpgradeSelected(true);
		*/
		//NotifyUpgradeSelected();
	}	
}

void AUpgradeAlternative::OnComponentEndOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex)
{
	if (IsTargetPlayer(OtherActor))
	{
		CurrentSelectionStatus = EUpgradeSelectionStatus::NotSelected;
		OnStatusChanged.Broadcast(CurrentSelectionStatus, Index);
		/*Server_NotifyUpgradeSelected(false);
		NotifyUpgradeSelected();*/
	}
}

void AUpgradeAlternative::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0)) //TODO: multiplayer
	{
		const FVector CameraLocation = PlayerController->PlayerCameraManager->GetCameraLocation();
		const FRotator LookAtRotation = (CameraLocation - GetActorLocation()).Rotation();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), LookAtRotation, DeltaTime, InterpSpeed));
	}
}

void AUpgradeAlternative::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUpgradeAlternative, UpgradeDisplayData);
	DOREPLIFETIME(AUpgradeAlternative, bUpgradeSelected);
	DOREPLIFETIME(AUpgradeAlternative, CurrentSelectionStatus);
	DOREPLIFETIME(AUpgradeAlternative, Index);
}
