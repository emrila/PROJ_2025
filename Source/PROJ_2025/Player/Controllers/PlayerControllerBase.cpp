#include "PlayerControllerBase.h"
#include "EnhancedInputSubsystems.h"
#include "RoomModifierBase.h"
#include "WizardGameInstance.h"
#include "WizardPlayerState.h"

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

void APlayerControllerBase::Server_SetLanPlayerName_Implementation(const FString& NewName)
{
	AWizardPlayerState* PS = GetPlayerState<AWizardPlayerState>();
	if (PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s, Player state found: %s"), *FString(__FUNCTION__), *PS->GetName());
		PS->LanPlayerName = NewName;
		//PS->ForceNetUpdate();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s, Player state not found."), *FString(__FUNCTION__));
	}
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		if (UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetGameInstance()))
		{
			if (GI->bIsLanGame)
			{
				Server_SetLanPlayerName(GI->LanPlayerName);
			}
		}
	}
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
