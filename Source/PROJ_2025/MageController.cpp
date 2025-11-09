

#include "MageController.h"

#include "Player/Components/AttackComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "MageCharacter.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

AMageController::AMageController()
{
}

void AMageController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMageController, ControlledCharacter);
}

void AMageController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMageController::Move(const FInputActionValue& Value)
{
	
	FVector2D MovementVector = Value.Get<FVector2D>();

	const FVector Direction = ControlledCharacter->GetActorForwardVector();
	const FVector RightVector = ControlledCharacter->GetActorRightVector();

	ControlledCharacter->AddMovementInput(Direction, MovementVector.Y);
	ControlledCharacter->AddMovementInput(RightVector, MovementVector.X);
	//DoMove(MovementVector.X, MovementVector.Y);
}

void AMageController::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AMageController::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this] ()
		{
			ControlledCharacter = Cast<ACharacter>(GetPawn());

			if (!ControlledCharacter)
			{
				UE_LOG(LogTemp, Error, TEXT("AMageController, Controlled Character is NULL!"));
				return;
			}

			MovementComponent = ControlledCharacter->GetCharacterMovement();

			if (!MovementComponent)
			{
				UE_LOG(LogTemp, Error, TEXT("AMageController, Character MoveComp is NULL"));
			}
		},
		0.1f,
		false
		);

	if (IsLocalPlayerController())
	{
		if (HUD)
		{
			HUD->AddToViewport();
		}
	}
}

void AMageController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	ControlledCharacter = nullptr;
	MovementComponent = nullptr;
}

void AMageController::SetupInputComponent_Implementation()
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

		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &AMageController::DoShoot);
	}
}

void AMageController::DoMove(float Right, float Forward)
{
	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AMageController::DoMove, Controlled Character is NULL!"));
		return;
	}

	/*const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	ControlledCharacter->AddMovementInput(ForwardDirection, Forward);
	ControlledCharacter->AddMovementInput(RightDirection, Right);*/

	const FVector Direction = ControlledCharacter->GetActorForwardVector();
	const FVector RightVector = ControlledCharacter->GetActorRightVector();

	ControlledCharacter->AddMovementInput(Direction, Forward);
	ControlledCharacter->AddMovementInput(RightVector, Right);
		
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

void AMageController::DoShoot()
{
	UE_LOG(LogTemp, Warning, TEXT("Shooting"));
	if (!ControlledCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageController, DoShoot, Controlled Character is NULL"));
		return;
	}
	
	AMageCharacter* MageCharacter = Cast<AMageCharacter>(ControlledCharacter);
	if (!MageCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageController, DoShoot, Mage Character is NULL"));
		return;
	}

	MageCharacter->GetAttackComponent()->StartAttack();
}

void AMageController::OnPossess_Implementation(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ControlledCharacter = Cast<AMageCharacter>(InPawn);

	SetupInputComponent_Implementation();
	
}

