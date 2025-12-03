#include "ShadowStrikeVariant2.h"

#include "EnemyBase.h"
#include "EnhancedInputComponent.h"
#include "Golem.h"
#include "KismetTraceUtils.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/Shield.h"

DEFINE_LOG_CATEGORY(ShadowStrikeLog);

UShadowStrikeVariant2::UShadowStrikeVariant2()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmount = 20.f;
	AttackCooldown = 10.f;
	// ...
}

void UShadowStrikeVariant2::StartAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (!bCanAttack)
	{
		UE_LOG(ShadowStrikeLog, Warning, TEXT("bCanAttack is false, checking for recast possibility."));
		if (!bShouldRecast)
		{
			UE_LOG(ShadowStrikeLog, Warning, TEXT("bShouldRecast is false. Returning without attacking."));
			return;
		}
	}

	if (!Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}

	TryLockingTargetOrLocation();

	if (!LockedTarget && LockedLocation.IsNearlyZero())
	{
		return;
	}

	float DistanceToTarget = FVector::Dist(LockedLocation, OwnerCharacter->GetActorLocation());

	if (DistanceToTarget <= MinimumDistanceToTarget)
	{
		return;
	}
	
	if (bShouldRecast)
	{
		UE_LOG(ShadowStrikeLog, Warning, TEXT("%s Attack was recast, resetting recast state."), *FString(__FUNCTION__));
		if (GetWorld()->GetTimerManager().IsTimerActive(RecastTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(RecastTimerHandle);
		}
		ResetRecast();
	}

	PerformAttack();

	Super::StartAttack();
}

void UShadowStrikeVariant2::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp,
                                                   UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this,
		                           &UShadowStrikeVariant2::OnPrepareForAttack);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this,
		                           &UShadowStrikeVariant2::OnLockedTarget);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Canceled, this,
		                           &UShadowStrikeVariant2::OnAttackCanceled);
	}
}

void UShadowStrikeVariant2::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Handle indicator here
	if (bHasLockedTarget && bCanAttack)
	{
		TryLockingTargetOrLocation();
		if (bCanTeleport/*!LockedLocation.IsNearlyZero()*/)
		{
			DrawDebugSphere(GetWorld(), LockedLocation, 150.f, 10, FColor::Cyan, false, 0.1f);
		}
	}

	//Not relevant anymore
	/*if (bKilledTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT(
			"%s Target was killed, AttackCooldown is reduced from: %f to %f."), *FString(__FUNCTION__), GetAttackCooldown(), GetAttackCooldown()/2.f);
		if (GetWorld()->GetTimerManager().IsTimerActive(AttackCooldownTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(AttackCooldownTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				AttackCooldownTimerHandle,
				this, 
				&UShadowStrikeVariant2::ResetAttackCooldown,
				GetAttackCooldown()/2.f,
				false);
		}
		bKilledTarget = false;
	}*/
}

void UShadowStrikeVariant2::BeginPlay()
{
	Super::BeginPlay();
}

void UShadowStrikeVariant2::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	if (LockedLocation.IsNearlyZero())
	{
		UE_LOG(LogTemp, Error, TEXT("%s LockedTarget is Null."), *FString(__FUNCTION__));
		return;
	}

	HandlePreAttackState();

	Server_TeleportPlayer();

	Server_PerformSweep();

	//Not relevant anymore
	/*FTimerHandle AttackTimer;
	GetWorld()->GetTimerManager().SetTimer(AttackTimer, [this, PlayerCharacter]()
	{
		if (!PlayerCharacter->GetFirstAttackComponent())
		{
			UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter's FirstAttackComponent is Null."), *FString(__FUNCTION__));
			return;
		}
		PlayerCharacter->GetFirstAttackComponent()->SetCanAttack(true);
		PlayerCharacter->GetFirstAttackComponent()->StartAttack(this->GetDamageAmount(), 1.f);
	}, StrikeDelay, false);*/

	FTimerHandle CameraReattachmentTimer;
	GetWorld()->GetTimerManager().SetTimer(
		CameraReattachmentTimer,
		this,
		&UShadowStrikeVariant2::HandlePostAttackState,
		StrikeDuration,
		false
	);
}

void UShadowStrikeVariant2::OnPrepareForAttack(const FInputActionInstance& ActionInstance)
{
	// Not sure if I need a parameter for this function and to actually compare ETriggerEvent types
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}

	bHasLockedTarget = true;
	PrepareForAttack();
}

void UShadowStrikeVariant2::OnLockedTarget(const FInputActionInstance& ActionInstance)
{
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}

	if (!bHasLockedTarget)
	{
		return;
	}

	if (OwnerCharacter->HasAuthority())
	{
		LockedLocation = FVector::ZeroVector;
		SweepStartLocation = FVector::ZeroVector;
	}
	else
	{
		Server_SetLockedLocation(FVector::ZeroVector, FVector::ZeroVector);
		LockedLocation = FVector::ZeroVector;
		SweepStartLocation = FVector::ZeroVector;
	}

	bHasLockedTarget = false;
	StartAttack();
}

//Not sure if this function is needed at all, can we cancel a shadow strike attack?
void UShadowStrikeVariant2::OnAttackCanceled(const FInputActionInstance& ActionInstance)
{
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Canceled)
	{
		return;
	}

	bHasLockedTarget = false;
}

void UShadowStrikeVariant2::PrepareForAttack()
{
	if (!Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}
	//TODO: Handle before attack animation here
	//UE_LOG(LogTemp, Warning, TEXT("I am preparing for the Shadow Strike Attack!"));
}

void UShadowStrikeVariant2::Server_SetLockedTarget_Implementation(AActor* Target)
{
	LockedTarget = Target;
}

void UShadowStrikeVariant2::Server_SetLockedLocation_Implementation(FVector Location, FVector SweepStart)
{
	LockedLocation = Location;
	SweepStartLocation = SweepStart;
}

void UShadowStrikeVariant2::HandlePreAttackState()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	OwnerCharacter->HandleCameraDetachment();
}

void UShadowStrikeVariant2::HandlePostAttackState()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	//LockedTarget = nullptr;
	if (OwnerCharacter->GetCharacterMovement())
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	OwnerCharacter->HandleCameraReattachment();
	if (!bWentThroughShield)
	{
		GetWorld()->GetTimerManager().SetTimer(PlayerIFrameTimer, [this]()
		{
			OwnerCharacter->ResetIFrame();
		}, 1.f, false);
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(PlayerIFrameTimer, [this]()
	{
		OwnerCharacter->ResetIFrame();
	}, 5.f, false);
}

void UShadowStrikeVariant2::Server_TeleportPlayer_Implementation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	if (LockedLocation.IsNearlyZero())
	{
		return;
	}

	const FVector CurrentTargetLocation = LockedLocation;
	const float CurrentTargetZ = CurrentTargetLocation.Z;

	const FVector CurrentPlayerLocation = OwnerCharacter->GetActorLocation();
	const FVector CurrentPlayerForward = OwnerCharacter->GetActorForwardVector();

	float DistanceToTarget = FVector::Dist(CurrentTargetLocation, CurrentPlayerLocation);

	if (LockedTarget)
	{
		if (Cast<AEnemyBase>(LockedTarget)->bIsDummy)
		{
			DistanceToTarget -= OffsetDistanceBehindTarget / 2.f;
		}
		else if (LockedTarget->IsA(AGolem::StaticClass()))
		{
			DistanceToTarget -= (OffsetDistanceBehindTarget + 100.f);
		}
		else
		{
			DistanceToTarget -= OffsetDistanceBehindTarget;
		}
	}

	FVector NewTargetLocation = CurrentPlayerForward * DistanceToTarget + CurrentPlayerLocation;
	NewTargetLocation.Z = CurrentTargetZ;

	// Play disappear effect at current player location
	DisappearLocation = CurrentPlayerLocation;
	Server_SpawnEffect_Implementation(DisappearLocation, DisappearEffect);

	// Cache player-to-camera offset for later camera interpolation (if a follow camera exists)
	FVector PlayerToCameraVector = FVector::ZeroVector;
	if (OwnerCharacter->GetFollowCamera())
	{
		PlayerToCameraVector = OwnerCharacter->GetFollowCamera()->GetComponentLocation() - CurrentPlayerLocation;
	}

	// Teleport on all clients
	Multicast_TeleportPlayer(NewTargetLocation);

	// Start camera interpolation relative to player's new location
	FVector NewCameraLocation = OwnerCharacter->GetActorLocation() + PlayerToCameraVector;
	OwnerCharacter->Client_StartCameraInterpolation(NewCameraLocation, CameraInterpDuration);
	/*FTimerHandle CameraLerpTimer;
	GetWorld()->GetTimerManager().SetTimer(CameraLerpTimer,
	                                       [this, PlayerToCameraVector]()
	                                       {
		                                       FVector NewCameraLocation = OwnerCharacter->GetActorLocation() +
			                                       PlayerToCameraVector;

		                                       OwnerCharacter->Client_StartCameraInterpolation(
			                                       NewCameraLocation, CameraInterpDuration);
	                                       },
	                                       CameraInterpDelay,
	                                       false
	);*/
}

void UShadowStrikeVariant2::Multicast_TeleportPlayer_Implementation(
	const FVector& TeleportLocation)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (LockedTarget)
	{
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
	}

	if (!OwnerCharacter->GetCapsuleComponent())
	{
		return;
	}

	/*float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();*/

	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		//OwnerCharacter->GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius / 5.f, CapsuleHalfHeight / 5.f, true);
		/*UE_LOG(LogTemp, Warning, TEXT("%s CapsuleRadius is: %f. Capsule HalfHeight is: %f"), *FString(__FUNCTION__),
			OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(), OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());*/
		OwnerCharacter->GetMesh()->SetHiddenInGame(true, true);
		OwnerCharacter->SetActorLocation(TeleportLocation, true, nullptr, ETeleportType::TeleportPhysics);
	}

	FTimerDelegate TimerDel = FTimerDelegate::CreateLambda([this]()
	{
		if (OwnerCharacter && OwnerCharacter->GetMesh())
		{
			Server_SpawnEffect_Implementation(AppearLocation, AppearEffect);
			OwnerCharacter->GetMesh()->SetHiddenInGame(false, true);
			//OwnerCharacter->GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight, true);
			/*UE_LOG(LogTemp, Warning, TEXT("%s CapsuleRadius is: %f. Capsule HalfHeight is: %f"), *FString(__FUNCTION__),
			OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(), OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());*/
		}
	});

	GetWorld()->GetTimerManager().SetTimer(PlayerTeleportTimerHandle, TimerDel, TeleportDelay, false);
}

void UShadowStrikeVariant2::TryLockingTargetOrLocation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (!OwnerCharacter->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s Pawn is not locally controlled; skipping camera-based locking."),
		       *FString(__FUNCTION__));
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

	TryLockingTarget(TraceStart, TraceEnd);

	if (LockedTarget)
	{
		bCanTeleport = true;
		return;
	}

	TryLockingLocation(TraceStart, TraceEnd);
}

void UShadowStrikeVariant2::TryLockingTarget(FVector StartLocation, FVector EndLocation)
{
	/*if (!OwnerCharacter)
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
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * GetAttackRange());*/

	if (!OwnerCharacter)
	{
		return;
	}

	FHitResult HitResult;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		StartLocation,
		EndLocation,
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
				LockedLocation = LockedTarget->GetActorLocation();
				SweepStartLocation = OwnerCharacter->GetActorLocation();
			}
			else
			{
				Server_SetLockedTarget(HitActor);
				Server_SetLockedLocation(LockedTarget->GetActorLocation(), OwnerCharacter->GetActorLocation());
				LockedTarget = HitActor;
				LockedLocation = LockedTarget->GetActorLocation();
				SweepStartLocation = OwnerCharacter->GetActorLocation();
			}
			bCanTeleport = true;
			return;
		}
	}
	LockedTarget = nullptr;
}

void UShadowStrikeVariant2::TryLockingLocation(FVector StartLocation, FVector EndLocation)
{
	if (!OwnerCharacter)
	{
		return;
	}

	FVector NewEndLocation = EndLocation;

	FVector Forward = OwnerCharacter->GetActorForwardVector();

	FVector ToTarget = (EndLocation - StartLocation).GetSafeNormal();

	float Dot = FVector::DotProduct(Forward, ToTarget);

	float AngleDegrees = FMath::RadiansToDegrees(acosf(FMath::Clamp(Dot, -1.f, 1.f)));

	//UE_LOG(LogTemp, Warning, TEXT("%s AngleDegrees is: %f."), *FString(__FUNCTION__), AngleDegrees);

	if (StartLocation.Z < EndLocation.Z)
	{
		if (AngleDegrees > AcceptableAngelDegrees)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s AngleDegrees is greater than AcceptableAngelDegrees."),
			       *FString(__FUNCTION__));
			const float TraceDistance = (EndLocation - StartLocation).Size();
			if (TraceDistance > KINDA_SMALL_NUMBER && AngleDegrees > KINDA_SMALL_NUMBER)
			{
				const float Fraction = AcceptableAngelDegrees / AngleDegrees;
				const FQuat DeltaQuat = FQuat::FindBetweenNormals(Forward, ToTarget);
				const FQuat ClampedQuat = FQuat::Slerp(FQuat::Identity, DeltaQuat, FMath::Clamp(Fraction, 0.f, 1.f));
				const FVector ClampedDir = ClampedQuat.RotateVector(Forward).GetSafeNormal();

				NewEndLocation = StartLocation + ClampedDir * TraceDistance;
			}
		}
	}
	else
	{
		NewEndLocation = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * LockOnRange;
	}

	if (OwnerCharacter->HasAuthority())
	{
		LockedLocation = NewEndLocation;
		SweepStartLocation = OwnerCharacter->GetActorLocation();
	}
	else
	{
		Server_SetLockedLocation(NewEndLocation, OwnerCharacter->GetActorLocation());
		LockedLocation = NewEndLocation;
		SweepStartLocation = OwnerCharacter->GetActorLocation();
	}
}

void UShadowStrikeVariant2::Server_SetWentThroughShield_Implementation(const bool Value)
{
	bWentThroughShield = Value;
}

void UShadowStrikeVariant2::Server_PerformSweep_Implementation()
{
	if (SweepStartLocation.IsNearlyZero())
	{
		return;
	}

	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	
	bool bHitEnemies = false;

	if (GetWorld())
	{
		bool bHit = GetWorld()->SweepMultiByObjectType(
			HitResults,
			SweepStartLocation,
			LockedLocation,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(50.f),
			Params
		);

		if (bHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				if (AActor* HitActor = Hit.GetActor())
				{
					if (HitActor->IsA(AEnemyBase::StaticClass()))
					{
						UGameplayStatics::ApplyDamage(
							HitActor,
							GetDamageAmount(),
							OwnerCharacter->GetController(),
							OwnerCharacter,
							UDamageType::StaticClass()
						);
						
						if (!bHitEnemies)
						{
							bHitEnemies = true;
							UE_LOG(ShadowStrikeLog, Warning, TEXT("%s, Hit Enemy"), *FString(__FUNCTION__));
						}
					}


					if (HitActor->IsA(AShield::StaticClass()))
					{
						Server_SetWentThroughShield_Implementation(true);
					}
				}
			}
		}

		/*if (bWentThroughShield)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s Went through shield, longer I-frames applied."), *FString(__FUNCTION__));
			DrawDebugSweptSphere(GetWorld(), SweepStartLocation, LockedLocation, 50.f, FColor::Green, false, 5.f, 0);
		}
		else
		{
			DrawDebugSweptSphere(GetWorld(), SweepStartLocation, LockedLocation, 50.f, FColor::Red, false, 5.f, 0);
		}*/
	}
	if (bHitEnemies)
	{
		UE_LOG(ShadowStrikeLog, Warning, TEXT("%s, Hit Enemy, Checking if this was a Recast attack."), *FString(__FUNCTION__));
		if (GetWorld()->GetTimerManager().IsTimerActive(RecastTimerHandle))
		{
			UE_LOG(ShadowStrikeLog, Warning, TEXT("%s, It was a Recast attack, cannot recast or attack before cooldown."), *FString(__FUNCTION__));
			return;
		}
		bShouldRecast = true;
	}
	else
	{
		UE_LOG(ShadowStrikeLog, Warning, TEXT("%s, No Hit, Setting bShouldRecast to false"), *FString(__FUNCTION__));
		bShouldRecast = false;
	}
	if (bShouldRecast)
	{
		UE_LOG(ShadowStrikeLog, Warning, TEXT("%s, It was not a recast, setting timer for recast attack possibility."), *FString(__FUNCTION__));
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			ResetRecast();
		}, RecastDuration, false);
	}
}

void UShadowStrikeVariant2::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	Server_SetLockedTarget_Implementation(nullptr);
	Server_SetWentThroughShield_Implementation(false);
	Server_SetLockedLocation_Implementation(FVector::ZeroVector, FVector::ZeroVector);
	DisappearLocation = FVector::ZeroVector;
	AppearLocation = FVector::ZeroVector;
	bWentThroughShield = false;
	bShouldRecast = false;
}

float UShadowStrikeVariant2::GetAttackCooldown() const
{
	if (AttackCooldown <= 5.f)
	{
		return 5.f;
	}
	return Super::GetAttackCooldown() * AttackSpeedModifier; // / AttackCooldown; ??👀 Was this a mistake?
}

float UShadowStrikeVariant2::GetDamageAmount() const
{
	if (FMath::IsNearlyEqual(AttackDamageModifier, 1.f, 0.0001f)) //if (AttackDamageModifier == 1.f)
	{
		return Super::GetDamageAmount();
	}
	return Super::GetDamageAmount() + (AttackDamageModifier * 20.f);
}

float UShadowStrikeVariant2::GetAttackRange() const
{
	/*if (AttackDamageModifier == 1.f)
	{
		return LockOnRange;
	}
	//500.f is random atm, TBD later
	return LockOnRange + (AttackDamageModifier * 500.f);*/
	return LockOnRange;
}
