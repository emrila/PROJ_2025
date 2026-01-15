// Furkan approves of this


#include "World/UpgradeAlternative.h"

#include "Components/InteractableComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Interfaces/Interactor.h"
#include "Net/UnrealNetwork.h"
#include "Util/UpgradeLog.h"

#include "World/UI/UpgradeAlternativeWidget.h"

AUpgradeAlternative::AUpgradeAlternative()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("UpgradeTriggerComponent"));

	SphereComponent->SetSphereRadius(50.f);

	SphereComponent->SetCollisionResponseToChannel(ECC_EngineTraceChannel2, ECR_Block); // ECC_GameTraceChannel2 = Interactable
	SphereComponent->SetupAttachment(RootComponent);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("UpgradeWidgetComponent"));
	WidgetComponent->SetupAttachment(RootComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(RootComponent);
	StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void AUpgradeAlternative::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->OnComponentBeginOverlap.AddDynamic(InteractableComponent.Get(), &UInteractableComponent::OnComponentBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(InteractableComponent.Get(), &UInteractableComponent::OnComponentEndOverlap);
}

void AUpgradeAlternative::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUpgradeAlternative, UpgradeDisplayData);
}

void AUpgradeAlternative::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (const ULocalPlayer* PlayerController = GetWorld()->GetFirstLocalPlayerFromController())
	{
		const FVector CameraLocation = PlayerController->GetPlayerController(GetWorld())->PlayerCameraManager->GetCameraLocation();
		const FRotator LookAtRotation = (CameraLocation - GetActorLocation()).Rotation();
		const FRotator TargetRotation = FRotator(LookAtRotation.Pitch, LookAtRotation.Yaw, GetActorRotation().Roll);

		SetActorRotation(TargetRotation);
	}
}

void AUpgradeAlternative::OnSetUpgradeDisplayData_Implementation(FInstancedStruct Data)
{
	const FUpgradeDisplayData* UpgradeData = Data.GetMutablePtr<FUpgradeDisplayData>();
	if (!UpgradeData)
	{
		UPGRADE_ERROR(TEXT("%hs: Data does not contain FUpgradeDisplayData!"), __FUNCTION__);
		return;
	}
	UpgradeDisplayData = *UpgradeData;
}

void AUpgradeAlternative::OnClearUpgradeDisplayData_Implementation()
{
	// Will call EndPlay which will then unregister from the owner's validation component
	 Destroy();
}

FInstancedStruct AUpgradeAlternative::OnGetUpgradeDisplayData_Implementation()
{
	return FInstancedStruct::Make(UpgradeDisplayData);
}

void AUpgradeAlternative::OnProcessUpgradeDisplayData_Implementation()
{
	if (!WidgetComponent || !WidgetComponent->GetWidget() ||!StaticMeshComponent)
	{
		UPGRADE_WARNING(TEXT("%hs: Still waiting -> WidgetComponent is %s, Widget is %s, staticMeshComponent is %s"), __FUNCTION__,
			WidgetComponent ? TEXT("valid") : TEXT("NULL"),
			WidgetComponent &&  WidgetComponent->GetWidget() ? TEXT("valid") : TEXT("NULL"),
			StaticMeshComponent ? TEXT("valid") : TEXT("NULL"));

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([this]
		{
			UPGRADE_DISPLAY(TEXT("%hs: Retrying OnProcessUpgradeDisplayData."), __FUNCTION__);
			Execute_OnProcessUpgradeDisplayData(this);
		}), 0.2f, false);
		return;
	}

	if (WidgetComponent->GetWidget()->Implements<UUpgradeDisplayInterface>())
	{
		if(Execute_OnGetOwner(this) != this)
		{
			Execute_OnSetOwner(WidgetComponent->GetWidget(), this);
		}
		Execute_OnSetUpgradeDisplayData(WidgetComponent->GetWidget(), Execute_OnGetUpgradeDisplayData(this));
	}

	if (UStaticMesh* NewMesh = UpgradeDisplayData.Mesh.LoadSynchronous())
	{
		StaticMeshComponent->SetStaticMesh(NewMesh);
	}
}

void AUpgradeAlternative::OnProcessSelectablesInfo_Implementation()
{
	if (!WidgetComponent || !WidgetComponent->GetWidget())
	{
		UPGRADE_WARNING(TEXT("%hs: Still waiting -> WidgetComponent is %s, Widget is %s"), __FUNCTION__,
			WidgetComponent ? TEXT("valid") : TEXT("NULL"),
		    WidgetComponent && WidgetComponent->GetWidget() ? TEXT("valid") : TEXT("NULL"));

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,  FTimerDelegate::CreateLambda([this]
		{
			UPGRADE_DISPLAY(TEXT("%hs: Retrying OnProcessSelectablesInfo."), __FUNCTION__);
			Execute_OnProcessSelectablesInfo(this);
		}), 0.2f, false);
		return;
	}

	if (WidgetComponent->GetWidget()->Implements<USelectionDisplayInterface>())
	{
		if(Execute_OnGetOwner(this) != this)
		{
			Execute_OnSetOwner(WidgetComponent->GetWidget(), this);
		}

		UPGRADE_DISPLAY(TEXT("%hs: Setting SelectablesInfo on WidgetComponent."), __FUNCTION__);
		const FInstancedStruct Data = Execute_OnGetSelectablesInfo(this, this);
		Execute_OnSetSelectablesInfo(WidgetComponent->GetWidget(), Data);
	}
}

void AUpgradeAlternative::OnRep_UpgradeDisplayData()
{
	UPGRADE_DISPLAY( TEXT("%hs: UpgradeDisplayData replicated %s."), __FUNCTION__, *UpgradeDisplayData.ToString());
	Execute_OnProcessUpgradeDisplayData(this);
}
