#include "ShadowStrikeAttackComp.h"

#include "EnemyBase.h"
#include "EnhancedInputComponent.h"
#include "Golem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"

#include "Player/Characters/PlayerCharacterBase.h"


UShadowStrikeAttackComp::UShadowStrikeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmount = 100.f;
	AttackCooldown = 5.f;

	// ...
}

void UShadowStrikeAttackComp::StartAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (!bCanAttack /*|| bIsLockingTarget*/)
	{
		return;
	}

	if (!Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}

	TryLockingTarget();

	if (LockedTarget)
	{
		PerformAttack();
		Super::StartAttack();
	}
}

void UShadowStrikeAttackComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp,
	UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &UShadowStrikeAttackComp::OnPrepareForAttack);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this, &UShadowStrikeAttackComp::OnLockedTarget);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Canceled, this, &UShadowStrikeAttackComp::OnAttackCanceled);
	}
}

void UShadowStrikeAttackComp::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bHasLockedTarget && bCanAttack)
	{
		TryLockingTarget();
		
		if (LockedTarget)
		{
			DrawDebugSphere(GetWorld(), LockedTarget->GetActorLocation(), 150.f, 10, FColor::Cyan, false, 0.1f);
		}
	}
	
	if (bKilledTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT(
			"%s Target was killed, AttackCooldown is reduced from: %f to %f."), *FString(__FUNCTION__), GetAttackCooldown(), GetAttackCooldown()/2.f);
		if (GetWorld()->GetTimerManager().IsTimerActive(AttackCooldownTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				AttackCooldownTimerHandle,
				this, 
				&UShadowStrikeAttackComp::ResetAttackCooldown,
				GetAttackCooldown()/2.f,
				false);
		}
		bKilledTarget = false;
	}
}

void UShadowStrikeAttackComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UShadowStrikeAttackComp::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	if (!LockedTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("%s LockedTarget is Null."), *FString(__FUNCTION__));
		return;
	}
	
	HandlePreAttackState();
	
	Server_TeleportPlayer();
	
	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerCharacter);
	
	if (!PlayerCharacter)	
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	FTimerHandle AttackTimer;
	GetWorld()->GetTimerManager().SetTimer(AttackTimer, [this, PlayerCharacter]()
	{
		if (!PlayerCharacter->GetFirstAttackComponent())
		{
			UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter's FirstAttackComponent is Null."), *FString(__FUNCTION__));
			return;
		}
		PlayerCharacter->GetFirstAttackComponent()->SetCanAttack(true);
		PlayerCharacter->GetFirstAttackComponent()->StartAttack(this->GetDamageAmount());
	}, StrikeDelay, false);
	
	
	FTimerHandle CameraReattachmentTimer;
	GetWorld()->GetTimerManager().SetTimer(
		CameraReattachmentTimer,
		this,
		&UShadowStrikeAttackComp::HandlePostAttackState,
		StrikeDuration,
		false
	);
	
	PlayerCharacter->StartIFrame();
	
	FTimerHandle PlayerIFrameTimer;
	GetWorld()->GetTimerManager().SetTimer(
		PlayerIFrameTimer,
		[PlayerCharacter]()
		{
			PlayerCharacter->ResetIFrame();
		},
		StrikeDuration + 1.f,
		false
	);
}

void UShadowStrikeAttackComp::OnPrepareForAttack(const FInputActionInstance& ActionInstance)
{
	// Not sure if I need a parameter for this function and to actually compare ETriggerEvent types
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}

	bHasLockedTarget = true;
	PrepareForAttack();
}

void UShadowStrikeAttackComp::OnLockedTarget(const FInputActionInstance& ActionInstance)
{
	// Not sure if I need a parameter for this function and to actually compare ETriggerEvent types
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}

	if (!bHasLockedTarget)
	{
		return;
	}
	
	
	bHasLockedTarget = false;
	StartAttack();
}

//Not sure if this function is needed at all
void UShadowStrikeAttackComp::OnAttackCanceled(const FInputActionInstance& ActionInstance)
{
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Canceled)
	{
		return;
	}

	bHasLockedTarget = false;
}

void UShadowStrikeAttackComp::PrepareForAttack()
{
	if (!Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}
	//TODO: Handle before attack animation here
	UE_LOG(LogTemp, Warning, TEXT("I am preparing for the Shadow Strike Attack!"));
}

void UShadowStrikeAttackComp::Server_SetLockedTarget_Implementation(AActor* Target)
{
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Locked Target is now set to null."), *FString(__FUNCTION__));
		LockedTarget = nullptr;
		return;
	}
	LockedTarget = Target;
	UE_LOG(LogTemp, Warning, TEXT("%s Locked Target is now set to %s."), *FString(__FUNCTION__), *LockedTarget->GetName());
}

void UShadowStrikeAttackComp::HandlePreAttackState()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetOwner());
	
	if (!PlayerCharacter)	
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	PlayerCharacter->HandleCameraDetachment();
}

void UShadowStrikeAttackComp::HandlePostAttackState()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetOwner());
	
	if (!PlayerCharacter)	
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	LockedTarget = nullptr;
	PlayerCharacter->HandleCameraReattachment();
}

void UShadowStrikeAttackComp::Server_TeleportPlayer_Implementation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	if (!LockedTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("%s LockedTarget is Null."), *FString(__FUNCTION__));
		return;
	}

	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerCharacter);

	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	const FVector CurrentTargetLocation = LockedTarget->GetActorLocation();
	const float CurrentTargetZ = CurrentTargetLocation.Z;

	FVector CurrentPlayerCameraLocation = FVector::ZeroVector;
	FVector CurrentPlayerCameraForwardVector = FVector::ZeroVector;

	if (PlayerCharacter->GetFollowCamera())
	{
		CurrentPlayerCameraLocation = PlayerCharacter->GetFollowCamera()->GetComponentLocation();
		CurrentPlayerCameraForwardVector = PlayerCharacter->GetFollowCamera()->GetForwardVector();
	}

	float DistanceToTarget = FVector::Dist(CurrentTargetLocation, CurrentPlayerCameraLocation);

	if (DistanceToTarget <= OffsetDistanceBehindTarget)
	{
		return;
	}

	if (LockedTarget->IsA(AGolem::StaticClass()))
	{
		DistanceToTarget -= (OffsetDistanceBehindTarget + 100.f);
	}
	else
	{
		DistanceToTarget -= OffsetDistanceBehindTarget;
	}

	FVector NewTargetLocation =
		CurrentPlayerCameraForwardVector * DistanceToTarget + CurrentPlayerCameraLocation;

	NewTargetLocation.Z = CurrentTargetZ;

	// TODO: Play VFX before teleporting the player
	DisappearLocation = PlayerCharacter->GetActorLocation();
	Server_SpawnEffect_Implementation(DisappearLocation, DisappearEffect);

	//Teleport player but cache current player location relativ to the camera
	FVector PlayerToCameraVector = CurrentPlayerCameraLocation - PlayerCharacter->GetActorLocation();
	
	Multicast_TeleportPlayer(NewTargetLocation);

	// TODO: Play VFX after teleporting the player

	//Interpolate camera
	FTimerHandle CameraLerpTimer;
	GetWorld()->GetTimerManager().SetTimer(CameraLerpTimer,
		[this, PlayerCharacter, PlayerToCameraVector]()
		{
			FVector NewCameraLocation = PlayerCharacter->GetActorLocation() + PlayerToCameraVector;

			PlayerCharacter->Client_StartCameraInterpolation(NewCameraLocation, CameraInterpDuration);
		},
		CameraInterpDelay,
		false
	);
}

void UShadowStrikeAttackComp::Multicast_TeleportPlayer_Implementation(
	const FVector& TeleportLocation)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (!LockedTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("%s LockedTarget is Null."), *FString(__FUNCTION__));
		return;
	}
	
	LockedTarget->CustomTimeDilation = 0.f;
	
	AEnemyBase* TheLockedTarget = Cast<AEnemyBase>(LockedTarget);
	
	if (TheLockedTarget)
	{
		if (TheLockedTarget->GetController())
		{
			TheLockedTarget->GetController()->CustomTimeDilation = 0.f;
		}
	}
	
	AppearLocation = TeleportLocation;
	
	if (TheLockedTarget)
	{
		GetWorld()->GetTimerManager().SetTimer(
				LockedTargetTickTimer,
				[this, TheLockedTarget]()
				{
					if (TheLockedTarget)
					{
						TheLockedTarget->CustomTimeDilation = 1.f;
						if (AController* C = TheLockedTarget->GetController())
						{
							C->CustomTimeDilation = 1.f;
						}
					}
				}, 2.5f, false);
	}
	
	ACharacter* LocalOwner = Cast<ACharacter>(GetOwner());
	if (LocalOwner && LocalOwner->GetMesh())
	{
		LocalOwner->GetMesh()->SetHiddenInGame(true, true);
	}

	TWeakObjectPtr<ACharacter> WeakOwner(LocalOwner);
	FTimerDelegate TimerDel = FTimerDelegate::CreateLambda([this, WeakOwner, TeleportLocation]()
	{
		if (ACharacter* Owner = WeakOwner.Get())
		{
			Owner->SetActorLocation(TeleportLocation, false, nullptr, ETeleportType::TeleportPhysics);
			if (Owner->GetMesh())
			{
				Server_SpawnEffect_Implementation(AppearLocation, AppearEffect);
				Owner->GetMesh()->SetHiddenInGame(false, true);
			}
		}
	});

	GetWorld()->GetTimerManager().SetTimer(PlayerTeleportTimerHandle, TimerDel, TeleportDelay, false);	
	/*if (AController* C = OwnerCharacter->GetController())
	{
		if (APlayerController* PC = Cast<APlayerController>(C))
		{
			if (PC->IsLocalController())
			{
				PC->SetControlRotation(TeleportRotation);
			}
		}
	}*/
}

void UShadowStrikeAttackComp::TryLockingTarget()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());

	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerController is Null."), *FString(__FUNCTION__));
		return;
	}

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;

	if (!CameraManager)
	{
		UE_LOG(LogTemp, Error, TEXT("%s CameraManager is Null."), *FString(__FUNCTION__));
		return;
	}

	FVector CameraLocation = CameraManager->GetCameraLocation();
	FRotator CameraRotation = CameraManager->GetCameraRotation();

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * GetAttackRange());


	FHitResult HitResult;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		TraceStart,
		TraceEnd,
		ObjectQueryParams,
		Params
		);

	/*if (!bHasLockedTarget)
	{
		DrawDebugLine(
		GetWorld(),
		TraceStart,
		TraceEnd,
		FColor::Red,
		false,
		5.0f,
		0,
		1.0f
		);
	}*/

	if (bHit && HitResult.GetActor())
	{
		if (AActor* HitActor = HitResult.GetActor(); !HitActor->IsA(APlayerCharacterBase::StaticClass()))
		{
			if (OwnerCharacter->HasAuthority())
			{
				LockedTarget = HitActor;
				//bIsLockingTarget = true;
			}
			else
			{
				Server_SetLockedTarget(HitActor);
				LockedTarget = HitActor;
				//bIsLockingTarget = true;
			}
			return;
		}
	}

	LockedTarget = nullptr;
	//bIsLockingTarget = false;
}

void UShadowStrikeAttackComp::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	Server_SetLockedTarget_Implementation(nullptr);
	DisappearLocation = FVector::ZeroVector;
	AppearLocation = FVector::ZeroVector;
}

float UShadowStrikeAttackComp::GetAttackCooldown() const
{
	return Super::GetAttackCooldown() * AttackSpeedModifier; // / AttackCooldown; ??👀 Was this a mistake?
}

float UShadowStrikeAttackComp::GetDamageAmount() const
{
	if (AttackDamageModifier == 1.f)
	{
		return Super::GetDamageAmount();
	}
	return Super::GetDamageAmount() + (AttackDamageModifier * 20.f);
}

float UShadowStrikeAttackComp::GetAttackRange() const
{
	if (AttackDamageModifier == 1.f)
	{
		return LockOnRange;
	}
	//500.f is random atm, TBD later
	return LockOnRange + (AttackDamageModifier * 500.f);
}
