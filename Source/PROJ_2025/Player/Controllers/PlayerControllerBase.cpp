#include "PlayerControllerBase.h"

#include "EnhancedInputSubsystems.h"
#include "../Characters/PlayerCharacterBase.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

APlayerControllerBase::APlayerControllerBase()
{
	ControlledPlayer = Cast<APlayerCharacterBase>(GetPawn());
}

void APlayerControllerBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerControllerBase, ControlledPlayer);
}

void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	if (!ControlledPlayer)
	{
		ControlledPlayer = Cast<APlayerCharacterBase>(GetPawn());
	}

	if (HealthBarWidget)
	{
		HealthBarWidget->AddToViewport();
	}
}

void APlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ControlledPlayer = nullptr;
}

void APlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledPlayer = Cast<APlayerCharacterBase>(GetPawn());

	SetupInputComponent_Implementation();
}

void APlayerControllerBase::SetupInputComponent_Implementation()
{
	Super::SetupInputComponent();

	if (!PlayerBaseMappingContext)
	{
		UE_LOG(LogTemp, Error, TEXT("APlayerControllerBase, PlayerBaseMappingContext is NULL"));
		return;
	}

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* LocalPlayerSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			LocalPlayerSubsystem->AddMappingContext(PlayerBaseMappingContext, 0);
		}
	}
}
