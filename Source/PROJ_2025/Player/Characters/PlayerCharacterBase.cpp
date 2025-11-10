#include "PlayerCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "WizardGameState.h"
#include "Camera/CameraComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"


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
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
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

