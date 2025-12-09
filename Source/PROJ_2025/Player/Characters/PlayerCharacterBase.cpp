#include "PlayerCharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "PlayerLoginSystem.h"
#include "WizardGameState.h"
#include "WizardPlayerState.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Core/UpgradeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Interact/Public/InteractorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/AttackComponentBase.h"
#include "Player/Controllers/PlayerControllerBase.h"
#include "Player/UI/PlayerNameTagWidget.h"


DEFINE_LOG_CATEGORY(PlayerBaseLog);

APlayerCharacterBase::APlayerCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Use the controller desired yaw rotation so the character rotates with mouse input
	bUseControllerRotationYaw = true;

	//Adjusting rotation rates, this can be changed in the editor to suit gameplay
	GetCharacterMovement()->RotationRate = FRotator(250.f, 250.0f, 250.0f);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	//FollowCamera->bUsePawnControlRotation = true;

	Tags.Add(TEXT("Player"));

	InteractorComponent = CreateDefaultSubobject<UInteractorComponent>(TEXT("InteractorComponent"));
	UpgradeComponent = CreateDefaultSubobject<UUpgradeComponent>(TEXT("UpgradeComponent"));
	
	PlayerNameTagWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerNameTagWidgetComponent"));
	PlayerNameTagWidgetComponent->SetupAttachment(RootComponent);
	PlayerNameTagWidgetComponent->AddLocalOffset(FVector(0.0f, 0.0f, 100.0f));
}

void APlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickNotLocal();
	
	if (bIsInterpolatingCamera && FollowCamera)
	{
		CameraInterpElapsed += DeltaTime;
		const float Alpha = FMath::Clamp(CameraInterpElapsed / FMath::Max(0.01f, CameraInterpDuration), 0.f, 1.f);

		const FVector NewLoc = FMath::Lerp(CameraInterpStartLocation, CameraInterpolateTargetLocation, Alpha);
		/*const FQuat StartQ = CameraInterpStartRotation.Quaternion();
		const FQuat EndQ = CameraInterpolateTargetRotation.Quaternion();
		const FQuat NewQ = FQuat::Slerp(StartQ, EndQ, Alpha);*/

		//FollowCamera->SetWorldLocationAndRotation(NewLoc, NewQ.Rotator());
		FollowCamera->SetWorldLocation(NewLoc);

		if (Alpha >= 1.f)
		{
			bIsInterpolatingCamera = false;
		}
	}

	if (IFrame)
	{
		// Possibly add visual effects or indicators for I-frames here
		//DrawDebugSphere(GetWorld(), GetActorLocation(), GetCapsuleComponent()->GetScaledCapsuleRadius(), 12, FColor::Green, false, 0.1f);
#if WITH_EDITOR
		DrawDebugSphere(GetWorld(), GetActorLocation(), 50.f, 12, FColor::Green, false, -0.1f, 0, 2.f);
#endif		
	}
}

void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (!PlayerInputComponent)
	{
		return;
	}
	
	if (!IsLocallyControlled())
	{
		return;
	}
	
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacterBase::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacterBase::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerCharacterBase::Look);
		
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacterBase::OnSprintBegin);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacterBase::OnSprintEnd);
		
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacterBase::Interact);
		
		
		FTimerHandle AttackComponentInputTimer;
		
		GetWorld()->GetTimerManager().SetTimer(
			AttackComponentInputTimer,
			[this, EnhancedInputComponent]()
			{
				SetupAttackComponentInput(EnhancedInputComponent);
			},
			1.f,
			false
			);
	}
}

UAttackComponentBase* APlayerCharacterBase::GetBasicAttackComponent() const
{
	return BasicAttackComponent;
}

UAttackComponentBase* APlayerCharacterBase::GetSpecialAttackComponent() const
{
	return SpecialAttackComponent;
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
	if (GetMesh())
	{
		TArray<AActor*> AttachedActors;
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
	//StartIFrame();
	
	bUseControllerRotationYaw = false;
	bShouldUseLookInput = false;
	bShouldUseMoveInput = false;
	
	FollowCamera->bUsePawnControlRotation = false;
	
	/*FollowCameraRelativeLocation = FollowCamera->GetRelativeLocation();
	FollowCameraRelativeRotation = FollowCamera->GetRelativeRotation();*/
	
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
	bShouldUseMoveInput = true;
	
	FollowCamera->bUsePawnControlRotation = true;
	
	FollowCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	FollowCamera->SetRelativeLocationAndRotation(FollowCameraRelativeLocation, FollowCameraRelativeRotation);
}

void APlayerCharacterBase::SetInputActive(const bool bNewInputActive)
{
	bIsInputActive = bNewInputActive;
}

void APlayerCharacterBase::StartIFrame()
{
	IFrame = true;
	OnIFrameStarted.Broadcast(IFrame);
}

void APlayerCharacterBase::ResetIFrame()
{
	IFrame = false;
	OnIFrameStarted.Broadcast(IFrame);
}

void APlayerCharacterBase::SetIsAlive(const bool NewIsAlive)
{
	UE_LOG(PlayerBaseLog, Log, TEXT("%s, NewIsAlive: %d"), *FString(__FUNCTION__), NewIsAlive);
	bIsAlive = NewIsAlive;
	OnPlayerDied.Broadcast(bIsAlive);
}

void APlayerCharacterBase::Client_StartCameraInterpolation_Implementation(const FVector& TargetLocation, const float LerpDuration)
{
	if (!FollowCamera)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, FollowCamera is Null"), *FString(__FUNCTION__));
		return;
	}
	
	/*FTransform TargetTransform;
	TargetTransform.SetLocation(TargetLocation);
	TargetTransform.SetRotation(TargetRotation.Quaternion());*/
	
	FVector ToTargetLocation = TargetLocation;
	InterpolateCamera(ToTargetLocation, LerpDuration);
}

void APlayerCharacterBase::Client_ShowDamageVignette_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !DamageVignetteWidget) return;

	UUserWidget* DamageVignette = CreateWidget<UUserWidget>(PC, DamageVignetteWidget);
	if (DamageVignette)
	{
		DamageVignette->AddToViewport();
	}
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	//Create objects for attack components
	/*if (BasicAttackComponentClass)
	{
		BasicAttackComponent = NewObject<UAttackComponentBase>(this, BasicAttackComponentClass);
		if (BasicAttackComponent)
		{
			BasicAttackComponent->RegisterComponent();
			BasicAttackComponent->InitializeComponent();
		}
	}
	
	if (SpecialAttackComponentClass)
	{
		SpecialAttackComponent = NewObject<UAttackComponentBase>(this, SpecialAttackComponentClass);
		
		if (SpecialAttackComponentClass)
		{
			SpecialAttackComponent->RegisterComponent();
			SpecialAttackComponent->InitializeComponent();
		}
	}*/
	
	if (FollowCamera)
	{
		FollowCameraRelativeLocation = FollowCamera->GetRelativeLocation();
		FollowCameraRelativeRotation = FollowCamera->GetRelativeRotation();
	}
	
	SetUpLocalCustomPlayerName();

	if (UpgradeComponent && IsLocallyControlled())
	{	
		UpgradeComponent->BindAttribute(GetMovementComponent(), TEXT("MaxWalkSpeed"), TEXT("MovementSpeed"));
		
		const FName AttackSpeedModifierPropName = "AttackSpeedModifier";
		const FName AttackDamageModifierPropName = "AttackDamageModifier";
		
		UpgradeComponent->BindAttribute(BasicAttackComponent, AttackSpeedModifierPropName, TEXT("BasicAttackSpeed"));
		UpgradeComponent->BindAttribute(BasicAttackComponent, AttackDamageModifierPropName, TEXT("BasicAttackDamage"));
		
		UpgradeComponent->BindAttribute(SpecialAttackComponent, AttackSpeedModifierPropName, TEXT("SpecialCooldown"));
		UpgradeComponent->BindAttribute(SpecialAttackComponent, AttackDamageModifierPropName, TEXT("SpecialDamage"));		

		if (AWizardGameState* GameState = GetWorld()->GetGameState<AWizardGameState>())
		{
			const FName MaxHealthPropName = "MaxHealth";
			const FName LifeStealMultiplierPropName = "LifeStealMultiplier";

			UpgradeComponent->BindAttribute(GameState, MaxHealthPropName, TEXT("PlayerMaxHealth"));
			UpgradeComponent->BindAttribute(GameState, LifeStealMultiplierPropName, TEXT("PlayerLifeSteal"));

			UE_LOG(PlayerBaseLog, Log, TEXT("Binding LifeStealMultiplier to MaxHealth changes"));
			if (FAttributeData* AttributeData = UpgradeComponent->GetByKey(GameState, GameState->GetClass()->FindPropertyByName(MaxHealthPropName)))
			{
				AttributeData->OnAttributeModified.AddWeakLambda(GameState, [GameState]
				{
					if (!GameState)
					{
						return;
					}
					UE_LOG(PlayerBaseLog, Log, TEXT("Updating LifeStealMultiplier to %f"), GameState->MaxHealth);
					GameState->SetMaxHealth(GameState->MaxHealth);
				});

			}
		}
	}

	if (InteractorComponent && !InteractorComponent->OnFinishedInteraction.IsAlreadyBound(UpgradeComponent, &UUpgradeComponent::OnUpgradeReceived))
	{		
	 	InteractorComponent->OnFinishedInteraction.AddDynamic(UpgradeComponent, &UUpgradeComponent::OnUpgradeReceived);
	}
}

void APlayerCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);


	if (UpgradeComponent && IsLocallyControlled())
	{
		UpgradeComponent->BindAttribute(GetMovementComponent(), TEXT("MaxWalkSpeed"), TEXT("MovementSpeed"));

		const FName AttackSpeedModifierPropName = "AttackSpeedModifier";
		const FName AttackDamageModifierPropName = "AttackDamageModifier";

		UpgradeComponent->BindAttribute(BasicAttackComponent, AttackSpeedModifierPropName, TEXT("BasicAttackSpeed"));
		UpgradeComponent->BindAttribute(BasicAttackComponent, AttackDamageModifierPropName, TEXT("BasicAttackDamage"));

		UpgradeComponent->BindAttribute(SpecialAttackComponent, AttackSpeedModifierPropName, TEXT("SpecialCooldown"));
		UpgradeComponent->BindAttribute(SpecialAttackComponent, AttackDamageModifierPropName, TEXT("SpecialDamage"));

		if (AWizardGameState* GameState = GetWorld()->GetGameState<AWizardGameState>())
		{
			const FName MaxHealthPropName = "MaxHealth";
			const FName LifeStealMultiplierPropName = "LifeStealMultiplier";

			UpgradeComponent->BindAttribute(GameState, MaxHealthPropName, TEXT("PlayerMaxHealth"));
			UpgradeComponent->BindAttribute(GameState, LifeStealMultiplierPropName, TEXT("PlayerLifeSteal"));

			UE_LOG(PlayerBaseLog, Log, TEXT("Binding LifeStealMultiplier to MaxHealth changes"));
			if (FAttributeData* AttributeData = UpgradeComponent->GetByKey(GameState, GameState->GetClass()->FindPropertyByName(MaxHealthPropName)))
			{
				AttributeData->OnAttributeModified.AddWeakLambda(GameState, [GameState]
				{
					if (!GameState)
					{
						return;
					}
					UE_LOG(PlayerBaseLog, Log, TEXT("Updating MaxHealth to %f"), GameState->MaxHealth);
					GameState->SetMaxHealth(GameState->MaxHealth);
				});

			}
			//UpgradeComponent->UpgradeByRow( TEXT("PlayerMaxHealth"));
			//UpgradeComponent->UpgradeByRow( TEXT("PlayerLifeSteal"));

		}
	}
}

void APlayerCharacterBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerCharacterBase, CustomPlayerName);
	DOREPLIFETIME(APlayerCharacterBase, bChangedName);
	DOREPLIFETIME(APlayerCharacterBase, bIsAlive);
	DOREPLIFETIME(APlayerCharacterBase, SuddenDeath);
	DOREPLIFETIME(APlayerCharacterBase, IFrame);
}

float APlayerCharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (!bIsAlive)
	{
		return 0.f;
	}
	const float NewDamageAmount = DamageAmount * DefenceStat;
	if (AWizardGameState* GameState = GetWorld()->GetGameState<AWizardGameState>(); !IFrame)
	{
		if (SuddenDeath)
		{
			SetIsAlive(false);
			GameState->OnRep_Health();
			return 0;
		}
		
		GameState->DamageHealth(NewDamageAmount);
		AWizardPlayerState* WizardPlayerState = Cast<AWizardPlayerState>(GetPlayerState());
		if (!WizardPlayerState)
		{
			return 0.f;
		}
		if (bIsAlive)
		{
			WizardPlayerState->AddDamageTaken(NewDamageAmount);
		}
		if (DamageAmount >= 10)
		{
			Client_ShowDamageVignette(); // send to owning client
			StartIFrame();
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &APlayerCharacterBase::ResetIFrame, 0.5, false);
		}
		Server_HitFeedback();
		return NewDamageAmount;
	}
	return 0;
}

void APlayerCharacterBase::TickNotLocal()
{
	if (IsLocallyControlled())
	{
		return;
	}
	const FVector ComponentLocation = PlayerNameTagWidgetComponent->GetComponentLocation();
	if (!GEngine)
	{
		return;
	}
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	if (LocalPlayerController && LocalPlayerController->PlayerCameraManager)
	{
		const FVector CameraLocation = LocalPlayerController->PlayerCameraManager->GetCameraLocation();
		const FRotator FindLookAtRotation = UKismetMathLibrary::FindLookAtRotation(ComponentLocation, CameraLocation);
		PlayerNameTagWidgetComponent->SetWorldRotation(FindLookAtRotation);
	}
}

void APlayerCharacterBase::InterpolateCamera(FVector& TargetLocation, const float LerpDuration)
{
	if (!FollowCamera)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, FollowCamera is Null"), *FString(__FUNCTION__));
		return;
	}
	
	CameraInterpStartLocation = FollowCamera->GetComponentLocation();
	//CameraInterpStartRotation = FollowCamera->GetComponentRotation();
	
	CameraInterpolateTargetLocation = TargetLocation;
	//CameraInterpolateTargetRotation = TargetTransform.Rotator();
	
	CameraInterpDuration = FMath::Max(0.01f, LerpDuration);
	CameraInterpElapsed = 0.f;
	bIsInterpolatingCamera = true;
}

void APlayerCharacterBase::Move(const FInputActionValue& Value)
{
	if (!bShouldUseMoveInput || !bIsAlive || !bIsInputActive)
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
	if (!bShouldUseLookInput || !bIsInputActive)
	{
		return;
	}	
	const FVector2D LookAxisVector = Value.Get<FVector2D>();	
	
	if (const APlayerControllerBase* PlayerController = Cast<APlayerControllerBase>(GetController()))
	{
		const float MouseSensitivity = PlayerController->GetMouseSensitivity();
		AddControllerYawInput(LookAxisVector.X * MouseSensitivity);
		AddControllerPitchInput(LookAxisVector.Y * MouseSensitivity);
		return;
	}
	
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacterBase::OnSprintBegin(const FInputActionInstance& ActionInstance)
{
	if (!bShouldUseSprintInput || !bIsInputActive) { return; }
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}
	
	if (!HasAuthority())
	{
		Server_SetSprint(true);
		bShouldSprint = true;
		if (GetCharacterMovement())
		{
			CurrentMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
			GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
		}
		return;
	}
	
	bShouldSprint = true;
	
	if (GetCharacterMovement())
	{
		CurrentMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
		GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
	}
}

void APlayerCharacterBase::OnSprintEnd(const FInputActionInstance& ActionInstance)
{
	if (!bShouldUseSprintInput || !bIsInputActive) { return; }
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}
	
	if (!HasAuthority())
	{
		Server_SetSprint(false);
		if (!bShouldSprint) { return; }
		bShouldSprint = false;
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->MaxWalkSpeed = CurrentMaxWalkSpeed;
		}
		return;
	}
	
	if (!bShouldSprint) { return; }
	bShouldSprint = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = CurrentMaxWalkSpeed;
	}
}

void APlayerCharacterBase::Interact(const FInputActionValue& Value)
{
	if (InteractorComponent && bIsAlive)
	{
        InteractorComponent->Execute_OnInteract(InteractorComponent,InteractorComponent->GetTargetInteractable().GetObject());
	}
}

void APlayerCharacterBase::SetupAttackComponentInput(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (!EnhancedInputComponent)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, EnhancedInputComponent is Null"), *FString(__FUNCTION__));
		return;
	}
	
	if (!BasicAttackComponent)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, FirstAttackComp is Null"), *FString(__FUNCTION__));
		return;
	}
	
	if (!BasicAttackAction)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, BasicAttackAction is Null"), *FString(__FUNCTION__));
		return;
	}
	
	BasicAttackComponent->SetupOwnerInputBinding(EnhancedInputComponent, BasicAttackAction);
	
	if (!SpecialAttackComponent)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, SecondAttackComp is Null"), *FString(__FUNCTION__));
		return;
	}
	
	if (!SpecialAttackAction)
	{
		UE_LOG(PlayerBaseLog, Error, TEXT("%s, SpecialAttackAction is Null"), *FString(__FUNCTION__));
		return;

	}
	SpecialAttackComponent->SetupOwnerInputBinding(EnhancedInputComponent, SpecialAttackAction);
}

void APlayerCharacterBase::Server_SetSprint_Implementation(const bool bNewSprintState)
{
	bShouldSprint = bNewSprintState;
	if (GetCharacterMovement())
	{
		if (bNewSprintState)
		{
			CurrentMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
			GetCharacterMovement()->MaxWalkSpeed *= SprintSpeedMultiplier;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = CurrentMaxWalkSpeed;
		}
	}
}

void APlayerCharacterBase::Server_SpawnEffect_Implementation(const FVector& EffectSpawnLocation)
{
}

void APlayerCharacterBase::Multicast_SpawnEffect_Implementation(const FVector& EffectSpawnLocation)
{
	
}

void APlayerCharacterBase::StartSuddenDeath()
{
	SuddenDeath = true;
}

void APlayerCharacterBase::EndSuddenDeath()
{
	SuddenDeath = false;
}

void APlayerCharacterBase::Jump()
{
	if (bIsAlive)
	{
		Super::Jump();
	}
}

void APlayerCharacterBase::Server_HitFeedback_Implementation()
{
	if (!HasAuthority())
	{
		return;
	}
	Multicast_HitFeedback();
}

void APlayerCharacterBase::Multicast_HitFeedback_Implementation()
{
	HitFeedback();
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

void APlayerCharacterBase::Server_SetCustomPlayerName_Implementation(const FString& InPlayerName)
{
	CustomPlayerName = InPlayerName;
	OnRep_CustomPlayerName();
}

bool APlayerCharacterBase::Server_SetCustomPlayerName_Validate(const FString& InPlayerName)
{
	return true;
}

void APlayerCharacterBase::SetUpLocalCustomPlayerName()
{
	if (!IsLocallyControlled())
	{
		return;
	}
	
	if (!GetPlayerState())
	{
		/*constexpr float InRate = 0.2f;
		UE_LOG(PlayerBaseLog, Warning, TEXT("%hs, PlayerState is Null, retrying in %f"), __FUNCTION__, InRate);
		FTimerHandle TimerHandle;

		GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		                                {
			                                SetUpLocalCustomPlayerName();
		                                },
		                                InRate, false);*/

		return;
	}

	const int32 PlayerId = GetPlayerState()->GetPlayerId(); //FMath::RandRange(10, 99);		
	InteractorComponent->Server_SetOwnerID(PlayerId);
	UE_LOG(PlayerBaseLog, Log, TEXT("%hs, Local player id: %d"), __FUNCTION__, PlayerId);

	FString NewName = FString::Printf(TEXT("Player_%d"), PlayerId);
	if (!bChangedName)
	{
/*#if WITH_EDITORONLY_DATA
		if (bUsePlayerLoginProfile)
		{
			if (const UPlayerLoginSystem* PlayerLoginSystem = GetGameInstance()->GetSubsystem<UPlayerLoginSystem>())
			{
				NewName = PlayerLoginSystem->GetProfile().Username;
			}
		}
#else
		if (UPlayerLoginSystem* PlayerLoginSystem = GetGameInstance()->GetSubsystem<UPlayerLoginSystem>())
		{
			NewName = PlayerLoginSystem->GetProfile().Username;
		}
#endif*/
		bChangedName = true;
	}
	else
	{
		UE_LOG(PlayerBaseLog, Log, TEXT("%hs, Player has changed name before, keeping existing name: %s"), __FUNCTION__, *CustomPlayerName);
	}
	Server_SetCustomPlayerName(NewName);	
	CustomPlayerName = NewName;
	OnRep_CustomPlayerName();
}
