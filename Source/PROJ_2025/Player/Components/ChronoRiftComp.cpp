#include "ChronoRiftComp.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"


UChronoRiftComp::UChronoRiftComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UChronoRiftComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UChronoRiftComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction)
{
	Super::SetupOwnerInputBinding(OwnerInputComp, OwnerInputAction);

	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this,
								   &UChronoRiftComp::OnStartLockingTargetArea);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this,
								   &UChronoRiftComp::OnTargetAreaLocked);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Canceled, this,
								   &UChronoRiftComp::OnStartLockingCanceled);
	}
}

void UChronoRiftComp::StartAttack()
{
	Super::StartAttack();
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (!bCanAttack /*|| bIsLockingTarget*/)
	{
		return;
	}

	TryLockingTargetArea();

	if (!TargetAreaCenter.IsNearlyZero())
	{
		PerformAttack();
		Super::StartAttack();
	}
}


void UChronoRiftComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

void UChronoRiftComp::PerformAttack()
{
	Super::PerformAttack();
}

void UChronoRiftComp::TryLockingTargetArea()
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
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult, 
		TraceStart, 
		TraceEnd, 
		ObjectQueryParams, 
		Params
		);
	
	if (bHit && HitResult.GetActor())
	{
		if (OwnerCharacter->HasAuthority())
		{
			TargetAreaCenter = HitResult.ImpactPoint;
		}
		else
		{
			Server_SetTargetAreaCenter_Implementation(HitResult.ImpactPoint);
			TargetAreaCenter = HitResult.ImpactPoint;
		}
		return;
	}
	TargetAreaCenter = FVector::ZeroVector;
}

void UChronoRiftComp::OnStartLockingTargetArea(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Canceled)
	{
		return;
	}
	bIsLockingTargetArea = true;
	PrepareForLaunch();
}

void UChronoRiftComp::OnTargetAreaLocked(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}

	if (!bIsLockingTargetArea)
	{
		return;
	}
	
	
	bIsLockingTargetArea = false;
	StartAttack();
}

void UChronoRiftComp::OnStartLockingCanceled(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Canceled)
	{
		return;
	}

	bIsLockingTargetArea = false;
}

void UChronoRiftComp::PrepareForLaunch()
{
	//Handle animation and spawning the circle here
	UE_LOG(LogTemp, Warning, TEXT("I am preparing for the Chrono Rift Launch!"));
}

void UChronoRiftComp::Server_PerformLaunch_Implementation()
{
}

void UChronoRiftComp::Server_SetTargetAreaCenter_Implementation(const FVector& TargetCenter)
{
	if (TargetCenter.IsNearlyZero())
	{
		UE_LOG(LogTemp, Error, TEXT("%s TargetCenter is Zero."), *FString(__FUNCTION__));
		TargetAreaCenter = TargetCenter;
		return;
	}
	
	TargetAreaCenter = TargetCenter;
	UE_LOG(LogTemp, Warning, TEXT("%s TargetCenter is set to %s"), *FString(__FUNCTION__), *TargetAreaCenter.ToString());
}



