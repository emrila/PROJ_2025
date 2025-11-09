#include "PlayerCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"


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

void APlayerCharacterBase::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FVector Direction = GetActorForwardVector();
	const FVector RightVector = GetActorRightVector();

	AddMovementInput(Direction, MovementVector.Y);
	AddMovementInput(RightVector, MovementVector.X);
}

void APlayerCharacterBase::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
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

		//EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AMageController::DoShoot);
	}
}

UAttackComponent* APlayerCharacterBase::GetFirstAttackComponent() const
{
	return FirstAttackComponent;
}

UAttackComponent* APlayerCharacterBase::GetSecondAttackComponent() const
{
	return SecondAttackComponent;
}

