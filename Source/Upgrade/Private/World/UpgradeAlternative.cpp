// Furkan approves of this


#include "World/UpgradeAlternative.h"

#include "Interactor.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Dev/UpgradeLog.h"
#include "Net/UnrealNetwork.h"
#include "World/UI/UpgradeAlternativeWidget.h"

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

	SetNetCullDistanceSquared(0.0); // Disable distance culling
	SetNetUpdateFrequency(10.0f); // Update more frequently
	SetMinNetUpdateFrequency(2.0f);

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("UpgradeWidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("UpgradeTriggerComponent"));
	SphereComponent->SetupAttachment(RootComponent);

	constexpr float SphereRadius = 50.f; //100.0f;
	SphereComponent->SetSphereRadius(SphereRadius);
	SphereComponent->SetRelativeLocation(FVector(0.0f, 0.0f, SphereRadius / 2.f));

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
		UpgradeWidget->OnSetUpgradeDisplayData(UpgradeDisplayData);
		UpgradeWidget->OnUpgradeSelected(bSelected);
	}
	if (bSelected && !bLocked)
	{
		OnUpgrade.Broadcast(UpgradeDisplayData);
	}
}

bool AUpgradeAlternative::IsTargetLocalPlayer(const AActor* OtherActor) const
{
	if (const APawn* Pawn = Cast<APawn>(OtherActor))
	{
		return Pawn->IsPlayerControlled() && Pawn->IsLocallyControlled();
	}
	return false;
}

void AUpgradeAlternative::OnRep_Selected()
{
	UPGRADE_DISPLAY(TEXT("%hs: bUpgradeSelected replicated to %s"), __FUNCTION__, bSelected ? TEXT("true") : TEXT("false"));	
	SelectUpgrade();
}

void AUpgradeAlternative::OnInteract_Implementation(UObject* Interactor)
{
	if (!HasAuthority())
	{
		UPGRADE_DISPLAY(TEXT("%hs: Client tried to interact! This should be handled on the server."), __FUNCTION__);
		return;
	}
	
	if (!Execute_CanInteract(this))
	{
		UPGRADE_DISPLAY(TEXT("%hs: Cannot interact right now!"), __FUNCTION__);
		return;
	}	
	
	const bool bIsInteractor = Interactor && Interactor->Implements<IInteractor::UClassType>();

	bSelected = true;
	UpgradeDisplayData.TargetName = bIsInteractor ? IInteractor::Execute_GetOwnerName(Interactor) : NAME_None;	
	SelectUpgrade();

	if (bIsInteractor)
	{
		UPGRADE_WARNING(TEXT("%hs: Interactor is null or doesn't implement IInteractor!"), __FUNCTION__);
		return;
	}
	
	UPGRADE_DISPLAY(TEXT("%hs: Notifying interactor of finished interaction."), __FUNCTION__);
	IInteractor::Execute_OnFinishedInteraction(Interactor, this);
		
	const FInstancedStruct InstancedStruct = FInstancedStruct::Make(UpgradeDisplayData);
	IInteractor::Execute_OnSuperFinishedInteraction(Interactor, InstancedStruct);
	//Destroy(); 
}

bool AUpgradeAlternative::CanInteract_Implementation()
{
	return bFocus && !bSelected && !bLocked;
}

void AUpgradeAlternative::OnPostInteract_Implementation()
{
	if (OnPostUpgrade.IsBound())
	{
		OnPostUpgrade.Broadcast();
	}
}

void AUpgradeAlternative::OnComponentBeginOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComp, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex,[[maybe_unused]] bool bFromSweep, [[maybe_unused]] const FHitResult& SweepResult)
{
	if (IsTargetLocalPlayer(OtherActor) && !bFocus && !bSelected && !bLocked)
	{
		SetFocus(true);
	}
}

void AUpgradeAlternative::OnComponentEndOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp,[[maybe_unused]] int32 OtherBodyIndex)
{	
	if (IsTargetLocalPlayer(OtherActor) && Execute_CanInteract(this))
	{
		SetFocus(false);
		/*bFocus = false;
		if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent))
		{
			UpgradeWidget->OnUpgradeHasFocus(bFocus);
		}*/
	}
}

void AUpgradeAlternative::SetLocked(const bool bToggle)
{
	bLocked = bToggle;
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent); !bSelected)
	{
		UpgradeWidget->OnUpgradeSelected(bSelected);
		SetFocus(false);
	}	
}

void AUpgradeAlternative::SetFocus(const bool bToggle)
{
	bFocus = bToggle;
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent))
	{
		UpgradeWidget->OnUpgradeHasFocus(bFocus);
	}
}

void AUpgradeAlternative::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (const ULocalPlayer* PlayerController = GetWorld()->GetFirstLocalPlayerFromController())
	{
		const FVector CameraLocation = PlayerController->GetPlayerController(GetWorld())->PlayerCameraManager->GetCameraLocation();
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
