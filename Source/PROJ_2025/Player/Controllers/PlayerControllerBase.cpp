#include "PlayerControllerBase.h"

#include "EnhancedInputSubsystems.h"
#include "RoomModifierBase.h"
#include "../Characters/PlayerCharacterBase.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"

APlayerControllerBase::APlayerControllerBase()
{
	
}

void APlayerControllerBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerControllerBase::Client_SetSpawnRotation_Implementation(const FRotator& NewRot)
{
	SetControlRotation(NewRot);
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalPlayerController())
	{
		SetupInputComponent();
	}
}

void APlayerControllerBase::Server_RegisterModifierClient_Implementation(URoomModifierBase* Modifier)
{
	if (!Modifier) return;

	Modifier->RegisterClient();
}

void APlayerControllerBase::SetupInputComponent()
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
