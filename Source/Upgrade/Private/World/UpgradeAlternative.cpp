// Furkan approves of this


#include "World/UpgradeAlternative.h"

#include "Interactor.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Dev/UpgradeLog.h"
#include "Net/UnrealNetwork.h"
#include "Util/UpgradeFunctionLibrary.h"
#include "World/UpgradeSpawner.h"

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

void AUpgradeAlternative::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUpgradeAlternative, UpgradeDisplayData);
	DOREPLIFETIME(AUpgradeAlternative, bSelected);
	DOREPLIFETIME(AUpgradeAlternative, Index);
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

void AUpgradeAlternative::BeginPlay()
{
	Super::BeginPlay();
	
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent))
	{
		UpgradeWidget->OnSetUpgradeDisplayData(UpgradeDisplayData);
	}
	
	bFocus = false;
	bLocked = false;
	if (HasAuthority())
	{
		bSelected = false;
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

bool AUpgradeAlternative::CanInteract_Implementation()
{
	return bFocus && !bSelected && !bLocked;
}

void AUpgradeAlternative::OnInteract_Implementation(UObject* Interactor)
{
	if (!HasAuthority())
	{
		UPGRADE_DISPLAY(TEXT("%hs: Client tried to interact! This should be handled on the server."), __FUNCTION__);
		return;
	}
	if (bSelected)
	{
		UPGRADE_WARNING( TEXT("%hs: Upgrade already selected!"), __FUNCTION__);
		return;
	}
	
	//DON'T CALL Execute_CanInteract(this);
	
	const bool bIsInteractor = Interactor && Interactor->Implements<IInteractor::UClassType>();

	bSelected = true;
	UpgradeDisplayData.TargetName = bIsInteractor ? IInteractor::Execute_GetOwnerName(Interactor) : NAME_None;	
	SelectUpgrade();
	if (OwningSpawner)
	{
		if (!OwningSpawner->GetUpgradeAlternativePairs().IsValidIndex(Index))
		{
			UPGRADE_ERROR(TEXT("%hs: Owning spawner's UpgradeAlternativePairs does not have a valid index %d!"), __FUNCTION__, Index);
			return;
		}
		FUpgradeAlternativePair& UpgradeAlternativePair = OwningSpawner->GetUpgradeAlternativePairs()[Index];
		UpgradeAlternativePair.SelectedByPlayers[Index] = true;
		UpgradeAlternativePair.LockedForPlayer[Index] = true;
	}
	
	//ForceNetUpdate();

	if (!bIsInteractor)
	{
		UPGRADE_WARNING(TEXT("%hs: Interactor is null or doesn't implement IInteractor!"), __FUNCTION__);
		return;
	}
	
	UPGRADE_DISPLAY(TEXT("%hs: Notifying interactor of finished interaction."), __FUNCTION__);
	IInteractor::Execute_OnFinishedInteraction(Interactor, this);
		
	const FInstancedStruct InstancedStruct = FInstancedStruct::Make(UpgradeDisplayData);
	IInteractor::Execute_OnSuperFinishedInteraction(Interactor, InstancedStruct);
}

void AUpgradeAlternative::SelectUpgrade()
{
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent))
	{
		UpgradeWidget->OnSetUpgradeDisplayData(UpgradeDisplayData);
		UpgradeWidget->OnUpgradeSelected(bSelected);
		//UpgradeWidget->OnUpgradeHasFocus(true);
	}
	
	if (bSelected && !bLocked)
	{
		OnUpgrade.Broadcast(UpgradeDisplayData);
	}
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
	if (UUpgradeFunctionLibrary::IsLocalPlayer(OtherActor) && !bFocus/* && !bSelected && !bLocked*/)
	{
		SetFocus(true);
	}
}

void AUpgradeAlternative::OnComponentEndOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp,[[maybe_unused]] int32 OtherBodyIndex)
{	
	if (UUpgradeFunctionLibrary::IsLocalPlayer(OtherActor) && bFocus)
	{
		SetFocus(false);	
	}
}

void AUpgradeAlternative::SetLocked(const bool bToggle)
{
	bLocked = bToggle;
	if (UUpgradeAlternativeWidget* UpgradeWidget = UpgradeWidget::Get(WidgetComponent); !bSelected)
	{
		UpgradeWidget->OnUpgradeSelected(bSelected);		
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

void AUpgradeAlternative::OnRep_Selected()
{
	UPGRADE_DISPLAY(TEXT("%hs: bUpgradeSelected replicated to %s"), __FUNCTION__, bSelected ? TEXT("true") : TEXT("false"));	
	SelectUpgrade();
}
