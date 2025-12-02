#include "ShadowStrikeAttackComp.h"

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


UShadowStrikeAttackComp::UShadowStrikeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmount = 20.f;
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

	PerformAttack();

	Super::StartAttack();
}

void UShadowStrikeAttackComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp,
                                                     UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this,
		                           &UShadowStrikeAttackComp::OnPrepareForAttack);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this,
		                           &UShadowStrikeAttackComp::OnLockedTarget);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Canceled, this,
		                           &UShadowStrikeAttackComp::OnAttackCanceled);
	}
}

void UShadowStrikeAttackComp::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Handle indicator here
	/*if (bHasLockedTarget && bCanAttack)
	{
		TryLockingTarget();
		
		if (LockedTarget)
		{
			DrawDebugSphere(GetWorld(), LockedTarget->GetActorLocation(), 150.f, 10, FColor::Cyan, false, 0.1f);
		}
	}*/


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
				&UShadowStrikeAttackComp::ResetAttackCooldown,
				GetAttackCooldown()/2.f,
				false);
		}
		bKilledTarget = false;
	}*/
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
	/*if (!LockedTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("%s LockedTarget is Null."), *FString(__FUNCTION__));
		return;
	}*/

	HandlePreAttackState();

	Server_TeleportPlayer();

	Server_PerformSweep();

	/*APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerCharacter);
	
	/*if (!PlayerCharacter)	
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}*/

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
		&UShadowStrikeAttackComp::HandlePostAttackState,
		StrikeDuration,
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
	//UE_LOG(LogTemp, Warning, TEXT("I am preparing for the Shadow Strike Attack!"));
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
}

void UShadowStrikeAttackComp::Server_SetLockedLocation_Implementation(FVector Location, FVector SweepStart)
{
	if (Location == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Locked Location is now set to zero."), *FString(__FUNCTION__));
		LockedLocation = FVector::ZeroVector;
		SweepStartLocation = FVector::ZeroVector;
		return;
	}
	LockedLocation = Location;
	SweepStartLocation = SweepStart;
}

void UShadowStrikeAttackComp::HandlePreAttackState()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	OwnerCharacter->HandleCameraDetachment();
}

void UShadowStrikeAttackComp::HandlePostAttackState()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	LockedTarget = nullptr;
	if (OwnerCharacter->GetCharacterMovement())
	{
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	OwnerCharacter->HandleCameraReattachment();
	if (!bWentThroughShield)
	{
		GetWorld()->GetTimerManager().SetTimer(PlayerIFrameTimer, [this] ()
		{
			OwnerCharacter->ResetIFrame();
		}, 1.f, false);
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(PlayerIFrameTimer, [this] ()
	{
		OwnerCharacter->ResetIFrame();
	}, 5.f, false);
}

void UShadowStrikeAttackComp::Server_TeleportPlayer_Implementation()
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

	FVector CurrentPlayerCameraLocation = FVector::ZeroVector;
	FVector CurrentPlayerCameraForwardVector = FVector::ZeroVector;

	if (OwnerCharacter->GetFollowCamera())
	{
		CurrentPlayerCameraLocation = OwnerCharacter->GetFollowCamera()->GetComponentLocation();
		CurrentPlayerCameraForwardVector = OwnerCharacter->GetFollowCamera()->GetForwardVector();
	}

	float DistanceToTarget = FVector::Dist(CurrentTargetLocation, CurrentPlayerCameraLocation);

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

	FVector NewTargetLocation =
		CurrentPlayerCameraForwardVector * DistanceToTarget + CurrentPlayerCameraLocation;

	NewTargetLocation.Z = CurrentTargetZ;

	// TODO: Play VFX before teleporting the player
	DisappearLocation = OwnerCharacter->GetActorLocation();
	Server_SpawnEffect_Implementation(DisappearLocation, DisappearEffect);

	//Teleport player but cache current player location relative to the camera
	FVector PlayerToCameraVector = CurrentPlayerCameraLocation - OwnerCharacter->GetActorLocation();

	Multicast_TeleportPlayer(NewTargetLocation);

	// TODO: Play VFX after teleporting the player

	//Interpolate camera
	FVector NewCameraLocation = OwnerCharacter->GetActorLocation() +
												   PlayerToCameraVector;

	OwnerCharacter->Client_StartCameraInterpolation(
		NewCameraLocation, CameraInterpDuration);
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

void UShadowStrikeAttackComp::Multicast_TeleportPlayer_Implementation(
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

	float CapsuleRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		OwnerCharacter->GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius/5.f, CapsuleHalfHeight/5.f, true);
		UE_LOG(LogTemp, Warning, TEXT("%s CapsuleRadius is: %f. Capsule HalfHeight is: %f"), *FString(__FUNCTION__),
			OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(), OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		OwnerCharacter->GetMesh()->SetHiddenInGame(true, true);
		OwnerCharacter->SetActorLocation(TeleportLocation, true, nullptr, ETeleportType::TeleportPhysics);
	}

	FTimerDelegate TimerDel = FTimerDelegate::CreateLambda([this, CapsuleRadius, CapsuleHalfHeight]()
	{
		if (OwnerCharacter && OwnerCharacter->GetMesh())
		{
			Server_SpawnEffect_Implementation(AppearLocation, AppearEffect);
			OwnerCharacter->GetMesh()->SetHiddenInGame(false, true);
			OwnerCharacter->GetCapsuleComponent()->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight, true);
			UE_LOG(LogTemp, Warning, TEXT("%s CapsuleRadius is: %f. Capsule HalfHeight is: %f"), *FString(__FUNCTION__),
			OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(), OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		}
	});

	GetWorld()->GetTimerManager().SetTimer(PlayerTeleportTimerHandle, TimerDel, TeleportDelay, false);
}

void UShadowStrikeAttackComp::TryLockingTargetOrLocation()
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

	TryLockingTarget(TraceStart, TraceEnd);

	if (LockedTarget)
	{
		bCanTeleport = true;
		return;
	}

	TryLockingLocation(TraceStart, TraceEnd);
}

void UShadowStrikeAttackComp::TryLockingTarget(FVector StartLocation, FVector EndLocation)
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
			bCanTeleport = true;
			if (OwnerCharacter->HasAuthority())
			{
				LockedTarget = HitActor;
				LockedLocation = LockedTarget->GetActorLocation();
				SweepStartLocation = StartLocation;
			}
			else
			{
				Server_SetLockedTarget(HitActor);
				Server_SetLockedLocation(LockedTarget->GetActorLocation(), StartLocation);
				LockedTarget = HitActor;
				LockedLocation = LockedTarget->GetActorLocation();
				SweepStartLocation = StartLocation;
			}
			return;
		}
	}
	LockedTarget = nullptr;
}

void UShadowStrikeAttackComp::TryLockingLocation(FVector StartLocation, FVector EndLocation)
{
	if (!OwnerCharacter)
	{
		return;
	}

	FVector Forward = OwnerCharacter->GetActorForwardVector();

	FVector ToTarget = (EndLocation - StartLocation).GetSafeNormal();

	float Dot = FVector::DotProduct(Forward, ToTarget);

	float AngleDegrees = FMath::RadiansToDegrees(acosf(FMath::Clamp(Dot, -1.f, 1.f)));

	UE_LOG(LogTemp, Warning, TEXT("%s AngleDegrees is: %f."), *FString(__FUNCTION__), AngleDegrees);

	float NewAcceptableAngelDegrees = AcceptableAngelDegrees;

	if (StartLocation.Z > EndLocation.Z)
	{
		NewAcceptableAngelDegrees = AcceptableAngelDegrees - 5.f;
	}

	if (AngleDegrees > NewAcceptableAngelDegrees)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s AngleDegrees is greater than AcceptableAngelDegrees."),
		       *FString(__FUNCTION__));
		return;
	}

	bCanTeleport = true;

	FHitResult HitResult;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	if (GetWorld())
	{
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult, StartLocation, EndLocation, ECC_Visibility, Params);

		if (bHit && HitResult.GetActor() && !HitResult.GetActor()->IsA(APlayerCharacterBase::StaticClass()))
		{
			if (OwnerCharacter->HasAuthority())
			{
				LockedLocation = HitResult.ImpactPoint;
				SweepStartLocation = StartLocation;
			}
			else
			{
				Server_SetLockedLocation(HitResult.ImpactPoint, StartLocation);
				LockedLocation = HitResult.ImpactPoint;
				SweepStartLocation = StartLocation;
			}
		}
		else
		{
			if (OwnerCharacter->HasAuthority())
			{
				LockedLocation = EndLocation;
				SweepStartLocation = StartLocation;
			}
			else
			{
				Server_SetLockedLocation(EndLocation, StartLocation);
				LockedLocation = EndLocation;
				SweepStartLocation = StartLocation;
			}
		}
	}
}

void UShadowStrikeAttackComp::Server_PerformSweep_Implementation()
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

		//DrawDebugSweptSphere(GetWorld(), SweepStartLocation, LockedLocation, 50.f, FColor::Red, false, 5.f, 0);

		if (bHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				if (AActor* HitActor = Hit.GetActor())
				{
					if (HitActor->IsA(AEnemyBase::StaticClass()))
					UGameplayStatics::ApplyDamage(
						HitActor,
						GetDamageAmount(),
						OwnerCharacter->GetController(),
						OwnerCharacter,
						UDamageType::StaticClass()
					);

					if (HitActor->ActorHasTag("Shield"))
					{
						bWentThroughShield = true;
					}
				}
			}
		}
	}
}

void UShadowStrikeAttackComp::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	Server_SetLockedTarget_Implementation(nullptr);
	Server_SetLockedLocation_Implementation(FVector::ZeroVector, FVector::ZeroVector);
	DisappearLocation = FVector::ZeroVector;
	AppearLocation = FVector::ZeroVector;
}

float UShadowStrikeAttackComp::GetAttackCooldown() const
{
	return Super::GetAttackCooldown() * AttackSpeedModifier; // / AttackCooldown; ??👀 Was this a mistake?
}

float UShadowStrikeAttackComp::GetDamageAmount() const
{
	if (FMath::IsNearlyEqual(AttackDamageModifier, 1.f, 0.0001f)) //if (AttackDamageModifier == 1.f)
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
