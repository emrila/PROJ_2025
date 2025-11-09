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

	// Add this to force replication to all clients
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
			UpgradeSubsystem->UpgradeByRow(FName("TestFloat")); //TODO: make dynamic
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

void AUpgradeAlternative::OnRep_UpgradeSelected()
{
	UPGRADE_DISPLAY(TEXT("%hs: bUpgradeSelected replicated to %s"), __FUNCTION__, bUpgradeSelected ? TEXT("true") : TEXT("false"));
	NotifyUpgradeSelected();
}

void AUpgradeAlternative::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsTargetPlayer(OtherActor))		
	{
		Server_NotifyUpgradeSelected(true);
		NotifyUpgradeSelected();
	}	
}

void AUpgradeAlternative::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsTargetPlayer(OtherActor))
	{
		Server_NotifyUpgradeSelected(false);
		NotifyUpgradeSelected();
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
}
