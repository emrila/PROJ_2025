#include "ShadowStrikeAttackComp.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

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
	
	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetOwner());
	
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
	}, 0.5f, false);
	
	FTimerHandle CameraReattachmentTimer;
	GetWorld()->GetTimerManager().SetTimer(
		CameraReattachmentTimer,
		this,
		&UShadowStrikeAttackComp::HandlePostAttackState,
		AttackDuration,
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

	const FTransform BehindTransform = GetTransformBehindLockedTarget();
	
	Multicast_TeleportPlayer(BehindTransform);
}

void UShadowStrikeAttackComp::Multicast_TeleportPlayer_Implementation(const FTransform BehindTransform)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	OwnerCharacter->SetActorLocationAndRotation(BehindTransform.GetLocation(), BehindTransform.GetRotation());
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
	
	UE_LOG(LogTemp, Warning, TEXT("%s Trying to lock target."), *FString(__FUNCTION__));

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

FTransform UShadowStrikeAttackComp::GetTransformBehindLockedTarget() const
{
	if (!LockedTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("%s LockedTarget is Null."), *FString(__FUNCTION__));
		return OwnerCharacter->GetActorTransform();
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








