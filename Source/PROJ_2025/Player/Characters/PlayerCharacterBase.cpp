#include "PlayerCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerLoginSystem.h"
#include "WizardGameState.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(PlayerBaseLog);

APlayerCharacterBase::APlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Use controller desired yaw rotation so the character rotates with mouse input
	bUseControllerRotationYaw = true;

	//Adjusting rotation rates, can be changed in the editor to suit gameplay
	GetCharacterMovement()->RotationRate = FRotator(250.f, 250.0f, 250.0f);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = true;

	Tags.Add(TEXT("Player"));

	PlayerNameWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerNameWidgetComponent"));
	PlayerNameWidgetComponent->SetupAttachment(RootComponent);

}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	SetCustomPlayerNameLocal();
}

void APlayerCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

float APlayerCharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	AWizardGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AWizardGameState>() : nullptr;
	if (GameState)
	{
		GameState->DamageHealth(DamageAmount);
		return DamageAmount;
	}
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void APlayerCharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FVector Direction = GetActorForwardVector();
	const FVector RightVector = GetActorRightVector();

	AddMovementInput(Direction, MovementVector.Y);
	AddMovementInput(RightVector, MovementVector.X);

	//FDamageEvent DamageEvent;
	//TakeDamage(5.0f, DamageEvent, GetController(), this); // For testing damage
}

void APlayerCharacterBase::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacterBase::UseFirstAttackComponent()
{
	UE_LOG(PlayerBaseLog, Warning, TEXT("APlayerCharacterBase::UseFirstAttackComponent called"));
}

void APlayerCharacterBase::UseSecondAttackComponent()
{
	UE_LOG(PlayerBaseLog, Warning, TEXT("APlayerCharacterBase::UseSecondAttackComponent called"));
}

void APlayerCharacterBase::OnRep_CustomPlayerName()
{
	if (!PlayerNameWidgetComponent && !PlayerNameWidgetComponent->GetWidget())
	{
		return;
	}
	if (UPlayerNameWidget* PlayerNameWidget = Cast<UPlayerNameWidget>(PlayerNameWidgetComponent->GetWidget()))
	{
		PlayerNameWidget->SetPlayerName(CustomPlayerName);
	}
	else
	{
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			OnRep_CustomPlayerName();
		}, 0.1f, false);
	}
}

void APlayerCharacterBase::SetCustomPlayerNameLocal()
{
	if (IsLocallyControlled())
	{
		return;
	}
	if (const UPlayerLoginSystem* PlayerLoginSystem = GetGameInstance()->GetSubsystem<UPlayerLoginSystem>())
	{
		if (!PlayerLoginSystem->GetProfile().Username.IsEmpty())
		{
			CustomPlayerName = FName(*PlayerLoginSystem->GetProfile().Username);
		}
	}
	if (CustomPlayerName == NAME_None)
	{
		const int32 RandomInt = FMath::RandRange(10, 99);
		CustomPlayerName = FName(*FString::FromInt(RandomInt));		
	}
	
	OnRep_CustomPlayerName();
	Server_SetCustomPlayerName(CustomPlayerName);
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerCharacterBase, CustomPlayerName);
}

void APlayerCharacterBase::Server_SetCustomPlayerName_Implementation(const FName& InPlayerName)
{
	CustomPlayerName = InPlayerName;
	OnRep_CustomPlayerName();//?
}

bool APlayerCharacterBase::Server_SetCustomPlayerName_Validate(const FName& InPlayerName)
{
	return true;
}

void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterBase::SetupPlayerInputComponent_Implementation(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(
			JumpAction, ETriggerEvent::Started, this, &APlayerCharacterBase::Jump);
		EnhancedInputComponent->BindAction(
			JumpAction, ETriggerEvent::Completed, this, &APlayerCharacterBase::StopJumping);

		EnhancedInputComponent->BindAction(
				MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Move);
		EnhancedInputComponent->BindAction(
			MouseLookAction, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Look);

		EnhancedInputComponent->BindAction(
			FirstAttackAction, ETriggerEvent::Started, this, &APlayerCharacterBase::UseFirstAttackComponent);

		EnhancedInputComponent->BindAction(
			SecondAttackAction, ETriggerEvent::Started, this, &APlayerCharacterBase::UseSecondAttackComponent);
	}
}

UAttackComponentBase* APlayerCharacterBase::GetFirstAttackComponent() const
{
	return FirstAttackComponent;
}

UAttackComponentBase* APlayerCharacterBase::GetSecondAttackComponent() const
{
	return SecondAttackComponent;
}

