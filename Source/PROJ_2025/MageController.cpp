

#include "MageController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"

AMageController::AMageController()
{
}

void AMageController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMageController::Move(const FInputActionValue& Value)
{
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	DoMove(MovementVector.X, MovementVector.Y);
}

void AMageController::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMageController::BeginPlay()
{
	Super::BeginPlay();

	ControlledCharacter = Cast<ACharacter>(GetPawn());

	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AMageController, Controlled Character is NULL!"));
	}

	MovementComponent = ControlledCharacter->GetCharacterMovement();

	if (!MovementComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("AMageController, Character MoveComp is NULL"));
	}
}

void AMageController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ControlledCharacter = nullptr;
	MovementComponent = nullptr;
}

void AMageController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!MageMappingContext)
	{
		UE_LOG(LogTemp, Error, TEXT("AMageController, MageMappingContext is NULL"));
		return;
	}

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* LocalPlayerSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			LocalPlayerSubsystem->AddMappingContext(MageMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent =
		Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(
			JumpAction, ETriggerEvent::Started, this, &AMageController::DoJumpStart);
		EnhancedInputComponent->BindAction(
			JumpAction, ETriggerEvent::Completed, this, &AMageController::DoJumpEnd);

		EnhancedInputComponent->BindAction(
			MoveAction, ETriggerEvent::Triggered, this, &AMageController::Move);
		EnhancedInputComponent->BindAction(
			MouseLookAction, ETriggerEvent::Triggered, this, &AMageController::Look);

		//EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMageController::Look);
	}
}

void AMageController::DoMove(float Right, float Forward)
{
	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AMageController::DoMove, Controlled Character is NULL!"));
		return;
	}

	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	// get right vector 
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// add movement 
	ControlledCharacter->AddMovementInput(ForwardDirection, Forward);
	ControlledCharacter->AddMovementInput(RightDirection, Right);
		
}

void AMageController::DoLook(float Yaw, float Pitch)
{
	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AMageController::DoLook, ControlledCharacter is Null"));
		return;
	}

	ControlledCharacter->AddControllerYawInput(Yaw);
	ControlledCharacter->AddControllerPitchInput(Pitch);
}

void AMageController::DoJumpStart()
{
	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AMageController::DoJumpStart, ControlledCharacter is Null"));
		return;
	}

	ControlledCharacter->Jump();
}

void AMageController::DoJumpEnd()
{
	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AMageController::DoJumpEnd, ControlledCharacter is Null"));
		return;
	}
	ControlledCharacter->StopJumping();
}
