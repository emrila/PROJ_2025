// Furkan approves of this


#include "Core/UpgradeAlternative.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/UpgradeAlternativeWidget.h"
#include "Dev/UpgradeLog.h"
#include "Kismet/GameplayStatics.h"

AUpgradeAlternative::AUpgradeAlternative()
{
	PrimaryActorTick.bCanEverTick = true;
	
	UpgradeWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("UpgradeWidgetComponent"));
	UpgradeTriggerComponent = CreateDefaultSubobject<USphereComponent>(TEXT("UpgradeTriggerComponent"));
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	RootComponent = SceneComponent;
	UpgradeTriggerComponent->SetupAttachment(RootComponent);
	UpgradeWidgetComponent->SetupAttachment(UpgradeTriggerComponent);

	constexpr float SphereRadius = 100.0f;
	UpgradeTriggerComponent->SetSphereRadius(SphereRadius);
	UpgradeTriggerComponent->SetRelativeLocation(FVector(0.0f, 0.0f, SphereRadius/2.f));
	
	UpgradeTriggerComponent->OnComponentBeginOverlap.AddDynamic(this, &AUpgradeAlternative::OnUpgradeTriggerBeginOverlap);
	UpgradeTriggerComponent->OnComponentEndOverlap.AddDynamic(this, &AUpgradeAlternative::OnComponentEndOverlap);
}

void AUpgradeAlternative::SetUpgradeDisplayData(const FUpgradeDisplayData& Data) const
{
	if (!UpgradeWidgetComponent || !UpgradeWidgetComponent->GetWidget())
	{
		return;
	}
	if (UUpgradeAlternativeWidget* UpgradeWidget = Cast<UUpgradeAlternativeWidget>(UpgradeWidgetComponent->GetWidget()))
	{
		UpgradeWidget->OnSetUpgradeDisplayData(Data);
	}
}

bool AUpgradeAlternative::IsTargetPlayer(const AActor* OtherActor) const
{
	return OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0);//TODO: multiplayer & Check more robustly		
}

void AUpgradeAlternative::OnUpgradeTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsTargetPlayer(OtherActor))		
	{
		OnUpgrade.Broadcast();
		UPGRADE_DISPLAY(TEXT("%hs: Upgrade alternative triggered by %s"), __FUNCTION__, *OtherActor->GetName());
	}	
}

void AUpgradeAlternative::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) 
	{
		OnUpgrade.Broadcast();
		UPGRADE_DISPLAY(TEXT("%hs: Upgrade alternative triggered by %s"), __FUNCTION__, *OtherActor->GetName());
		Destroy(); //TODO: ObjectPool
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
