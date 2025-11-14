#include "PlayerCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerLoginSystem.h"
#include "WizardGameState.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interact/Public/InteractorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/AttackComponentBase.h"
#include "Player/UI/PlayerNameTagWidget.h"


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

	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("InteractorComponent"));
	PlayerNameTagWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerNameTagWidgetComponent"));
	PlayerNameTagWidgetComponent->SetupAttachment(RootComponent);
	PlayerNameTagWidgetComponent->AddLocalOffset(FVector(0.0f, 0.0f, 100.0f));
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetUpLocalCustomPlayerName();
}

void APlayerCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

float APlayerCharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	AWizardGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AWizardGameState>() : nullptr;
	if (GameState)
	{
		GameState->DamageHealth(DamageAmount);
		return DamageAmount;
	}
	
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void APlayerCharacterBase::InterpolateCameraToLocation(FVector& TargetLocation, const float LerpDuration)
{
}

void APlayerCharacterBase::InterpolateCameraToRotation(FRotator& TargetRotation, const float LerpDuration)
{
}

void APlayerCharacterBase::Move(const FInputActionValue& Value)
{
	if (!bShouldUseMovementInput)
	{
		return;
	}
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FVector Direction = GetActorForwardVector();
	const FVector RightVector = GetActorRightVector();

	AddMovementInput(Direction, MovementVector.Y);
	AddMovementInput(RightVector, MovementVector.X);
}

void APlayerCharacterBase::Look(const FInputActionValue& Value)
{
	if (!bShouldUseLookInput)
	{
		return;
	}
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacterBase::UseFirstAttackComponent()
{
	if (!FirstAttackComponent)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("APlayerCharacterBase::UseFirstAttackComponent, FirstAttackComp is Null"));
		return;
	}

	GetFirstAttackComponent()->StartAttack();
}

void APlayerCharacterBase::UseSecondAttackComponent()
{
	if (!SecondAttackComponent)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("APlayerCharacterBase::UseSecondAttackComponent, SecondAttackComp is Null"));
		return;
	}
	GetSecondAttackComponent()->StartAttack();
}

void APlayerCharacterBase::Interact(const FInputActionValue& Value)
{
	if (InteractorComponent)
	{
        InteractorComponent->Execute_OnInteract(InteractorComponent,InteractorComponent->GetTargetInteractable().GetObject());
	}
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerCharacterBase, CustomPlayerName);
	DOREPLIFETIME(APlayerCharacterBase, bChangedName);
}

void APlayerCharacterBase::OnRep_CustomPlayerName()
{
	if (!PlayerNameTagWidgetComponent)
	{
		return;
	}
	
	if (!PlayerNameTagWidgetComponent->GetWidget())
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			OnRep_CustomPlayerName();
		}, 0.1f, false);
		return;
	}
	
	if (UPlayerNameTagWidget* PlayerNameTagWidget = Cast<UPlayerNameTagWidget>(PlayerNameTagWidgetComponent->GetWidget()))
	{
		PlayerNameTagWidget->SetCustomPlayerName(FText::FromString(CustomPlayerName));
	}
}

void APlayerCharacterBase::SetUpLocalCustomPlayerName()
{
	if (!bChangedName)
	{
#if WITH_EDITORONLY_DATA
		if (!bUsePlayerLoginProfile)
		{
			const int32 RandomNum = FMath::RandRange(10, 99);
			CustomPlayerName = FString::Printf(TEXT("Player_%d"), RandomNum);
		}
		else if (UPlayerLoginSystem* PlayerLoginSystem = GetGameInstance()->GetSubsystem<UPlayerLoginSystem>())
		{
			CustomPlayerName = PlayerLoginSystem->GetProfile().Username;
		}
#else
		if (UPlayerLoginSystem* PlayerLoginSystem = GetGameInstance()->GetSubsystem<UPlayerLoginSystem>())
		{
			CustomPlayerName = PlayerLoginSystem->GetProfile().Username;
		}
#endif
		bChangedName = true;
}
		if (IsLocallyControlled())
		{
			Server_SetCustomPlayerName(CustomPlayerName);			
		}
		OnRep_CustomPlayerName();
	
}

void APlayerCharacterBase::Multicast_SpawnHitParticles_Implementation()
{
}

void APlayerCharacterBase::Server_SpawnHitParticles_Implementation()
{
}

void APlayerCharacterBase::TickNotLocal()
{
	if (!IsLocallyControlled())
	{
		const FVector ComponentLocation = PlayerNameTagWidgetComponent->GetComponentLocation();
		const FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->K2_GetActorLocation();
		const FRotator FindLookAtRotation = UKismetMathLibrary::FindLookAtRotation(ComponentLocation,CameraLocation);
		PlayerNameTagWidgetComponent->SetWorldRotation(FindLookAtRotation);
	}
}

void APlayerCharacterBase::Server_SetCustomPlayerName_Implementation(const FString& InPlayerName)
{
	CustomPlayerName = InPlayerName;
	OnRep_CustomPlayerName();
}

bool APlayerCharacterBase::Server_SetCustomPlayerName_Validate(const FString& InPlayerName)
{
	return true;
}

void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacterBase::SetupPlayerInputComponent_Implementation(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent)
	{
		return;
	}
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacterBase::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacterBase::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Look);

		EnhancedInputComponent->BindAction(FirstAttackAction, ETriggerEvent::Started, this, &APlayerCharacterBase::UseFirstAttackComponent);
		EnhancedInputComponent->BindAction(SecondAttackAction, ETriggerEvent::Started, this, &APlayerCharacterBase::UseSecondAttackComponent);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacterBase::Interact);
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

FVector APlayerCharacterBase::GetRightHandSocketLocation() const
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (MeshComp->DoesSocketExist(RightHandSocket))
		{
			return MeshComp->GetSocketLocation(RightHandSocket);
		}
	}
	return FVector::ZeroVector;
}

FVector APlayerCharacterBase::GetLeftHandSocketLocation() const
{
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (MeshComp->DoesSocketExist(LeftHandSocket))
		{
			return MeshComp->GetSocketLocation(LeftHandSocket);
		}
	}
	return FVector::ZeroVector;
}

AActor* APlayerCharacterBase::GetRightHandAttachedActor() const
{
	TArray<AActor*> AttachedActors;

	if (GetMesh())
	{
		GetMesh()->GetOwner()->GetAttachedActors(AttachedActors);
		for (AActor* Attached: AttachedActors)
		{
			if (Attached)
			{
				if (Attached->GetAttachParentSocketName() == RightHandSocket)
				{
					return Attached;
				}
			}
		}
	}
	UE_LOG(PlayerBaseLog, Warning, TEXT("%s, No actor attached to right hand socket"), *FString(__FUNCTION__));
	return nullptr;
}

void APlayerCharacterBase::HandleCameraDetachment()
{
	//bUseControllerRotationYaw = bUseControllerYaw;
	if (!CameraBoom)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, CameraBoom is Null"), *FString(__FUNCTION__));
		return;
	}
	
	if (!FollowCamera)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, FollowCamera is Null"), *FString(__FUNCTION__));
		return;
	}
	
	bUseControllerRotationYaw = false;
	bShouldUseLookInput = false;
	bShouldUseMovementInput = false;
	
	FollowCameraRelativeLocation = FollowCamera->GetRelativeLocation();
	FollowCameraRelativeRotation = FollowCamera->GetRelativeRotation();
	
	FollowCamera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void APlayerCharacterBase::HandleCameraReattachment()
{
	if (!CameraBoom)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, CameraBoom is Null"), *FString(__FUNCTION__));
		return;
	}
	
	if (!FollowCamera)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, FollowCamera is Null"), *FString(__FUNCTION__));
		return;
	}
	
	bUseControllerRotationYaw = true;
	bShouldUseLookInput = true;
	bShouldUseMovementInput = true;
	
	FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	FollowCamera->SetRelativeLocationAndRotation(FollowCameraRelativeLocation, FollowCameraRelativeRotation);
}

void APlayerCharacterBase::InterpolateCamera(
	FTransform& TargetTransform, const float LerpDuration)
{
	if (!FollowCamera)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, FollowCamera is Null"), *FString(__FUNCTION__));
		return;
	}
	
	FVector TargetLocation = TargetTransform.GetLocation();
	
	InterpolateCameraToLocation(TargetLocation, LerpDuration / 2.f);
	
}




















