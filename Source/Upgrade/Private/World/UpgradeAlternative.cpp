// Furkan approves of this


#include "World/UpgradeAlternative.h"

#include "Interactor.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
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

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("UpgradeWidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("UpgradeTriggerComponent"));
	SphereComponent->SetupAttachment(RootComponent);

	constexpr float SphereRadius = 50.f;//100.0f;
	SphereComponent->SetSphereRadius(SphereRadius);
	SphereComponent->SetRelativeLocation(FVector(0.0f, 0.0f, SphereRadius/2.f));
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AUpgradeAlternative::OnComponentBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AUpgradeAlternative::OnComponentEndOverlap);
}

void AUpgradeAlternative::SetUpgradeDisplayData(const FUpgradeDisplayData& Data)
{
	UpgradeDisplayData = Data;
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent))
	{
		UpgradeWidget->OnSetUpgradeDisplayData(UpgradeDisplayData);
	}
}

void AUpgradeAlternative::SelectUpgrade()
{
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent))
	{
		UpgradeWidget->OnUpgradeSelected(bSelected);
	}
	if (bSelected && !bLocked)
	{
		OnUpgrade.Broadcast(UpgradeDisplayData);		
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

void AUpgradeAlternative::Server_SelectUpgrade_Implementation(bool bIsSelected)
{
	bSelected = bIsSelected;
	SelectUpgrade();
}

bool AUpgradeAlternative::Server_SelectUpgrade_Validate(bool bIsSelected)
{
	return true;
}

void AUpgradeAlternative::OnRep_Selected()
{
	UPGRADE_DISPLAY(TEXT("%hs: bUpgradeSelected replicated to %s"), __FUNCTION__, bSelected ? TEXT("true") : TEXT("false"));
	SelectUpgrade();
}

void AUpgradeAlternative::OnInteract_Implementation(UObject* Interactor)
{	
	bSelected = true;
	
	Server_SelectUpgrade(bSelected);
	SelectUpgrade();
	
	if (Interactor && Interactor->Implements<IInteractor::UClassType>())
	{		
		IInteractor::Execute_OnFinishedInteraction(Interactor, this);
	}
	//Destroy(); 
}

bool AUpgradeAlternative::CanInteract_Implementation()
{
	return bFocus && !bSelected && !bLocked; 
}

void AUpgradeAlternative::OnComponentBeginOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComp, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex, [[maybe_unused]] bool bFromSweep, [[maybe_unused]] const FHitResult& SweepResult)
{
	if (IsTargetPlayer(OtherActor) && !bSelected && !bLocked)
	{				
		bFocus = true;
		if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent))
		{
			UpgradeWidget->OnUpgradeHasFocus(bFocus);
		}
	}	
}

void AUpgradeAlternative::OnComponentEndOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex)
{
	if (IsTargetPlayer(OtherActor) && !bSelected && !bLocked)
	{		
		bFocus = false;
		if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent))
		{
			UpgradeWidget->OnUpgradeHasFocus(bFocus);
		}		
	}
}

void AUpgradeAlternative::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0)) 
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
	DOREPLIFETIME(AUpgradeAlternative, bSelected);
	DOREPLIFETIME(AUpgradeAlternative, Index);
}
