#include "ChronoRiftComp.h"

#include "ChronoRiftDamageType.h"
#include "EnemyBase.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"


UChronoRiftComp::UChronoRiftComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UChronoRiftComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLockingTargetArea && bCanAttack)
	{
		TryLockingTargetArea();
		DrawDebugSphere(GetWorld(), TargetAreaCenter, TargetAreaRadius, 5, FColor::Yellow, false, 0.1);
	}
}

void UChronoRiftComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction)
{
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
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (!bCanAttack)
	{
		return;
	}

	if (!Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}

	bShouldLaunch = true;
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
	Server_PerformLaunch_Implementation();
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
	
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	//Line trace to find the ground location
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
		
		//DrawDebugSphere(GetWorld(), TargetAreaCenter, TargetAreaRadius, 5, FColor::Yellow, false, AttackCooldown);
		if (!bShouldLaunch)
		{
			return;
		}
		
		//Sphere sweep to find enemies in radius
		FCollisionObjectQueryParams EnemyQueryParams;
		EnemyQueryParams.AddObjectTypesToQuery(ECC_Pawn);
		const bool bEnemiesHit = GetWorld()->SweepMultiByObjectType(
			HitResults,
			TargetAreaCenter,
			TargetAreaCenter,
			FQuat::Identity,
			EnemyQueryParams,
			FCollisionShape::MakeSphere(TargetAreaRadius),
			Params
			);
		
		if (bEnemiesHit)
		{
			TArray<AActor*> HitActors;
			
			for (const FHitResult& Hit : HitResults)
			{
				if (Hit.GetActor() && !HitActors.Contains(Hit.GetActor()))
				{
					if (Hit.GetActor()->IsA(APlayerCharacterBase::StaticClass()))
					{
						continue;
					}
					HitActors.Add(Hit.GetActor());
				}
			}
			
			if (HitActors.Num() > 0)
			{
				if (OwnerCharacter->HasAuthority())
				{
					LockedTargets = HitActors;
				}
				else
				{
					Server_SetLockedEnemies_Implementation(HitActors);
					LockedTargets = HitActors;
				}
			}
		}
		return;
	}
	TargetAreaCenter = FVector::ZeroVector;
}

void UChronoRiftComp::OnStartLockingTargetArea(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
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
	if (!Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}
	//Handle animation and spawning the circle here
	UE_LOG(LogTemp, Warning, TEXT("I am preparing for the Chrono Rift Launch!"));
}

void UChronoRiftComp::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	bShouldLaunch = false;
	bIsLockingTargetArea = false;
	LockedTargets.Empty();
	TargetAreaCenter = FVector::ZeroVector;
	GetWorld()->GetTimerManager().ClearTimer(TickDamageTimerHandle);
}

void UChronoRiftComp::TickDamage_Implementation()
{
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (LockedTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s No Locked Targets to launch!"), *FString(__FUNCTION__));
		return;
	}
	
	for (AActor* Target: LockedTargets)
	{
		if (IsValid(Target))
		{
			UGameplayStatics::ApplyDamage(
				Target,
				DamageAmount,
				OwnerCharacter->GetController(),
				OwnerCharacter,
				UChronoRiftDamageType::StaticClass()
				);
			
			UE_LOG(LogTemp, Warning, TEXT("%s dealt %f damage to %s"), *FString(__FUNCTION__), DamageAmount, *Target->GetName());
		}
	}
}

void UChronoRiftComp::Server_SetLockedEnemies_Implementation(const TArray<AActor*>& Enemies)
{
	if (Enemies.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChronoRiftComp, Server_SetLockaedEnemies, Enemies array is empty!"));
		LockedTargets.Empty();
		return;
	}
	
	LockedTargets = Enemies;
}

void UChronoRiftComp::Server_PerformLaunch_Implementation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (LockedTargets.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s No Locked Targets to launch!"), *FString(__FUNCTION__));
		return;
	}
	
	for (AActor* Target: LockedTargets)
	{
		if (IsValid(Target))
		{
			if (AEnemyBase* Enemy = Cast<AEnemyBase>(Target))
			{
				Enemy->CustomTimeDilation = EnemyTimeDilationFactor;
				if (Enemy->GetController())
				{
					Enemy->GetController()->CustomTimeDilation = EnemyTimeDilationFactor;
				}
			}
		}
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		ResetEnemiesTimerHandle, 
		[this] ()
		{
			for (AActor* Target: LockedTargets)
			{
				if (IsValid(Target))
				{
					if (AEnemyBase* Enemy = Cast<AEnemyBase>(Target))
					{
						Enemy->CustomTimeDilation = 1.f;
						if (Enemy->GetController())
						{
							Enemy->GetController()->CustomTimeDilation = 1.f;
						}
					}
				}
			}
		}, ChronoDuration, false);
	
	
	GetWorld()->GetTimerManager().SetTimer(
		TickDamageTimerHandle,
		this, 
		&UChronoRiftComp::TickDamage,
		DamageTickInterval,
		true);
	
	FTimerHandle ClearTickDamageTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(ClearTickDamageTimerHandle, [this]()
	{
		GetWorld()->GetTimerManager().ClearTimer(TickDamageTimerHandle);
	}, ChronoDuration, false);
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



