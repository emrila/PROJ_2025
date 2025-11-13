#include "ShadowStrikeAttackComp.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
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
	Super::PerformAttack();

	if (!LockedTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("%s LockedTarget is Null."), *FString(__FUNCTION__));
		return;
	}

	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(GetOwner());

	if (!PlayerCharacter)	
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (!PlayerCharacter->GetSecondAttackComponent())
	{
		UE_LOG(LogTemp, Error, TEXT("%s SecondAttackComponent is Null."), *FString(__FUNCTION__));
		return;
	}

	HandlePlayerCameraDuringAttack();


	FTransform BehindTransform = GetLocationBehindLockedTarget();
	PlayerCharacter->SetUseControllerYawRotation(false);
	PlayerCharacter->SetActorLocationAndRotation(BehindTransform.GetLocation(), BehindTransform.GetRotation());

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

	FTimerHandle ResetRotationTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ResetRotationTimerHandle, [PlayerCharacter]()
	{
		PlayerCharacter->SetUseControllerYawRotation(true);
	}, AttackDuration, false);

	LockedTarget = nullptr;
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

void UShadowStrikeAttackComp::HandlePlayerCameraDuringAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerCharacter);

	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	UCameraComponent* FollowCamera = PlayerCharacter->GetFollowCamera();
	if (!FollowCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("%s FollowCamera is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (!LockedTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s LockedTarget is Null."), *FString(__FUNCTION__));
		return;
	}

	FVector CameraLocation = FollowCamera->GetRelativeLocation();
	FRotator CameraRotation = FollowCamera->GetRelativeRotation();

	
	FollowCamera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	FTimerHandle CameraTimerHandle;

	GetWorld()->GetTimerManager().SetTimer(CameraTimerHandle, [PlayerCharacter, CameraLocation, CameraRotation]()
	{
		PlayerCharacter->GetFollowCamera()->AttachToComponent(
			PlayerCharacter->GetCameraBoom(), FAttachmentTransformRules::SnapToTargetIncludingScale, USpringArmComponent::SocketName);
		PlayerCharacter->GetFollowCamera()->SetRelativeLocation(CameraLocation);
		PlayerCharacter->GetFollowCamera()->SetRelativeRotation(CameraRotation);
		//PlayerCharacter->GetFollowCamera()->SetupAttachment( PlayerCharacter->GetCameraBoom(), USpringArmComponent::SocketName);
	}, 2.f, false);

	
	/*const FVector TargetLocation = LockedTarget->GetActorLocation();
	const FVector TargetForward = LockedTarget->GetActorForwardVector();

	// You can tweak these offsets to taste
	const float CameraDistanceBehindTarget = 300.f;
	const float CameraHeightOffset = 100.f;

	FVector DesiredCameraLocation = TargetLocation - TargetForward * CameraDistanceBehindTarget;
	DesiredCameraLocation.Z += CameraHeightOffset;

	// Camera should face the target
	FRotator DesiredCameraRotation = (TargetLocation - DesiredCameraLocation).Rotation();

	// --- 3. Start a timed lerp to that position ---
	// Using a timer for simplicity (you could also do this in Tick)
	const float LerpDuration = 0.5f; // how long to blend to target view
	const float LerpInterval = 0.01f;
	float Elapsed = 0.f;

	FTimerHandle LerpTimerHandle;
	FVector StartLoc = FollowCamera->GetComponentLocation();
	FRotator StartRot = FollowCamera->GetComponentRotation();

	FTimerDelegate LerpDelegate;
	LerpDelegate.BindLambda([=, &Elapsed]() mutable
	{
		Elapsed += LerpInterval;
		float Alpha = FMath::Clamp(Elapsed / LerpDuration, 0.f, 1.f);

		FVector NewLoc = FMath::Lerp(StartLoc, DesiredCameraLocation, Alpha);
		FRotator NewRot = FMath::Lerp(StartRot, DesiredCameraRotation, Alpha);

		FollowCamera->SetWorldLocationAndRotation(NewLoc, NewRot);

		// When complete, stop the timer
		if (Alpha >= 1.f)
		{
			PlayerCharacter->GetWorldTimerManager().ClearTimer(LerpTimerHandle);
		}
	});

	PlayerCharacter->GetWorldTimerManager().SetTimer(LerpTimerHandle, LerpDelegate, LerpInterval, true);

	// --- 4. Reattach camera after attack ---
	const float TotalAttackDuration = 5.5f; // whatever your animation length is
	FTimerHandle ResetCameraHandle;
	PlayerCharacter->GetWorldTimerManager().SetTimer(ResetCameraHandle, [=]()
	{
		FollowCamera->AttachToComponent(
			PlayerCharacter->GetCameraBoom(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			USpringArmComponent::SocketName
			);
		FollowCamera->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);

	}, TotalAttackDuration, false);*/
}


/*void UShadowStrikeAttackComp::HandlePlayerCameraDuringAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	const APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerCharacter);

	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	UCameraComponent* FollowCamera = PlayerCharacter->GetFollowCamera();

	if (!FollowCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("%s FollowCamera is Null."), *FString(__FUNCTION__));
		return;
	}
	
	//Detach camera or change transform relative to world from player
	//Lerp camera to follow behind target while always facing the target
	//Reattach camera to player after attack
}*/

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








