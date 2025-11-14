#include "ShadowStrikeAttackComp.h"

#include "GameFramework/Character.h"

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
	
	UE_LOG(LogTemp, Warning, TEXT("%s Trying to lock target."), *FString(__FUNCTION__));

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
	
	HandlePlayerCamera();

	LockedTarget = nullptr;
}

void UShadowStrikeAttackComp::HandlePlayerCamera()
{
if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetOwner());
	
	if (!PlayerCharacter)	
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	PlayerCharacter->HandleCameraDetachment();

	FTransform BehindTransform = GetLocationBehindLockedTarget();
	PlayerCharacter->SetActorLocationAndRotation(BehindTransform.GetLocation(), BehindTransform.GetRotation());

	if (!PlayerCharacter->GetSecondAttackComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("%s SecondAttackComponent is Null."), *FString(__FUNCTION__));
		return;
	}
	PlayerCharacter->GetFirstAttackComponent()->SetCanAttack(true);// Ensure first attack can be used
	
	// Delay slightly to allow for position update before starting attack
	FTimerHandle AttackTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(AttackTimerHandle, [PlayerCharacter]()
	{
		if (PlayerCharacter->GetFirstAttackComponent())
		{
			PlayerCharacter->GetFirstAttackComponent()->StartAttack();
		}
	}, 0.5f, false);
	
	
	// Handle reattaching player camera after attack duration
	FTimerHandle CameraReattachmentTimer;
	GetWorld()->GetTimerManager().SetTimer(CameraReattachmentTimer, [PlayerCharacter]()
	{
		if (PlayerCharacter)
		{
			PlayerCharacter->HandleCameraReattachment();
		}
	}, 
	AttackDuration,
	false
	);
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
			LockedTarget = HitActor;
			bIsLockingTarget = true;
			return;
		}
	}

	LockedTarget = nullptr;
	bIsLockingTarget = false;
	
}

FTransform UShadowStrikeAttackComp::GetLocationBehindLockedTarget() const
{
	if (!LockedTarget)
	{
		return FTransform();
	}

	const FVector TargetLocation = LockedTarget->GetActorLocation();
	const FRotator TargetRotation = LockedTarget->GetActorRotation();

	const FVector TargetForward = TargetRotation.Vector();
	const FVector LocationBehind = TargetLocation - (TargetForward * OffsetDistanceBehindTarget);
	

	return FTransform(TargetRotation, LocationBehind);
}

void UShadowStrikeAttackComp::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	bIsLockingTarget = false;
}








