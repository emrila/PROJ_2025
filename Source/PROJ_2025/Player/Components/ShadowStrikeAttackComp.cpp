#include "ShadowStrikeAttackComp.h"

#include "EnemyBase.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

#include "Player/Characters/PlayerCharacterBase.h"


UShadowStrikeAttackComp::UShadowStrikeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UShadowStrikeAttackComp::StartAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (!bCanAttack || bIsLockingTarget)
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
	GetWorld()->GetTimerManager().SetTimer(AttackTimer, [PlayerCharacter]()
	{
		PlayerCharacter->GetFirstAttackComponent()->SetCanAttack(true);
		PlayerCharacter->GetFirstAttackComponent()->StartAttack();
	}, AttackDelay, false);
	
	
	FTimerHandle CameraReattachmentTimer;
	GetWorld()->GetTimerManager().SetTimer(
		CameraReattachmentTimer,
		this,
		&UShadowStrikeAttackComp::HandlePostAttackState,
		StrikeDuration,
		false
	);
}

void UShadowStrikeAttackComp::Server_SetLockedTarget_Implementation(AActor* Target)
{
	if (!Target)
	{
		UE_LOG(LogTemp, Error, TEXT("%s Target is Null."), *FString(__FUNCTION__));
		return;
	}
	LockedTarget = Target;
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

	//Commented out to try a location in front of the tareget instead
	/*const FVector TeleportLocation = LockedTarget->GetActorLocation();
	const FRotator TeleportRotation = LockedTarget->GetActorRotation();
	const FVector TargetForward = TeleportRotation.Vector();
	const FVector LocationBehind = TeleportLocation - (TargetForward * OffsetDistanceBehindTarget);*/
	
	const FVector TeleportLocation = LockedTarget->GetActorLocation();
	const FRotator TargetRotation = LockedTarget->GetActorRotation();
	const FVector TargetForward = TargetRotation.Vector();
	const FVector LocationInFront = TeleportLocation + (TargetForward * OffsetDistanceBehindTarget);
	const FRotator OppositeRotation = TargetRotation + FRotator(0.f, 180.f, 0.f);
	
	
	//Multicast_TeleportPlayer(LocationBehind, TeleportRotation);
	
	Multicast_TeleportPlayer(LocationInFront, OppositeRotation);

	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerCharacter);
	
	if (!PlayerCharacter)	
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	/*const FVector CameraTargetLocation = 
		LocationBehind - (TargetForward * CameraInterpDistanceBehind) + FVector(0.f, 0.f, CameraInterpHeight
			);
	
	const FRotator CameraTargetRotation = 
		UKismetMathLibrary::FindLookAtRotation(CameraTargetLocation, LocationBehind);
	
	PlayerCharacter->Client_StartCameraInterpolation(CameraTargetLocation, CameraTargetRotation, CameraInterpDuration);*/
	
	const FVector PlayerForward = OppositeRotation.Vector();
	const FVector CameraTargetLocation = LocationInFront - (PlayerForward * CameraInterpDistanceBehind) + FVector(0.f, 0.f, CameraInterpHeight);
	const FRotator CameraTargetRotation = OppositeRotation;

	PlayerCharacter->Client_StartCameraInterpolation(CameraTargetLocation, CameraTargetRotation, CameraInterpDuration);
}

void UShadowStrikeAttackComp::Multicast_TeleportPlayer_Implementation(
	const FVector& TeleportLocation, const FRotator& TeleportRotation)
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
		//TheLockedTarget->CustomTimeDilation = 0.f;
		TheLockedTarget->GetController()->CustomTimeDilation = 0.f;
	}
	
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
	
	OwnerCharacter->SetActorLocationAndRotation(
		TeleportLocation, TeleportRotation, false, nullptr, ETeleportType::TeleportPhysics);
	
	if (AController* C = OwnerCharacter->GetController())
	{
		if (APlayerController* PC = Cast<APlayerController>(C))
		{
			if (PC->IsLocalController())
			{
				PC->SetControlRotation(TeleportRotation);
			}
		}
	}
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
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * LockOnRange);


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

	if (bHit && HitResult.GetActor())
	{
		if (AActor* HitActor = HitResult.GetActor(); !HitActor->IsA(APlayerCharacterBase::StaticClass()))
		{
			if (OwnerCharacter->HasAuthority())
			{
				LockedTarget = HitActor;
				bIsLockingTarget = true;
			}
			else
			{
				Server_SetLockedTarget(HitActor);
				LockedTarget = HitActor;
				bIsLockingTarget = true;
			}
			return;
		}
	}

	LockedTarget = nullptr;
	bIsLockingTarget = false;
}

void UShadowStrikeAttackComp::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	bIsLockingTarget = false;
}








