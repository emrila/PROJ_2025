#include "ChronoRiftComp.h"

#include "ChronoRiftDamageType.h"
#include "EnemyBase.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/ChronoRiftZone.h"


UChronoRiftComp::UChronoRiftComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);

	DamageAmount = 2.f;
	AttackCooldown = 15.f;

	// ...
}

void UChronoRiftComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLockingTargetArea && bCanAttack)
	{
		TryLockingTargetArea();
		if (TargetAreaCenter != FVector::ZeroVector)
		{
			if (LovesMesh)
			{
				FVector SpawnLocation = FVector(TargetAreaCenter.X, TargetAreaCenter.Y, TargetAreaCenter.Z + 5.f);
				LovesMesh->SetActorLocation(SpawnLocation);
				//Set Scale
			}
		}
		else
		{
			if (LovesMesh)
			{
				LovesMesh->SetActorHiddenInGame(true);
			}
		}
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

	if (TargetAreaCenter != FVector::ZeroVector)
	{
		PerformAttack();
		Super::StartAttack();
	}
}

void UChronoRiftComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UChronoRiftComp, LockedTargets);
	DOREPLIFETIME(UChronoRiftComp, TargetAreaCenter);
	DOREPLIFETIME(UChronoRiftComp, LovesMesh);
	DOREPLIFETIME(UChronoRiftComp, CurrentChronoRiftZone);
}

void UChronoRiftComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	if (ChronoRiftIndicatorClass)
	{
		LovesMesh = GetWorld()->SpawnActor<AActor>(ChronoRiftIndicatorClass, TargetAreaCenter, FRotator::ZeroRotator);
		
		if (LovesMesh)
		{
			LovesMesh->SetActorHiddenInGame(true);
		}
	}
}

void UChronoRiftComp::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	Server_PerformLaunch();
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
			Server_SetTargetAreaCenter(HitResult.ImpactPoint);
			TargetAreaCenter = HitResult.ImpactPoint;
		}
		
		//DrawDebugSphere(GetWorld(), TargetAreaCenter, GetAttackRadius(), 5, FColor::Yellow, false, AttackCooldown);
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
			FCollisionShape::MakeSphere(GetAttackRadius()),
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
					Server_SetLockedEnemies(HitActors);
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
	if (LovesMesh)
	{
		LovesMesh->SetActorHiddenInGame(false);
	}
	UE_LOG(LogTemp, Warning, TEXT("I am preparing for the Chrono Rift Launch!"));
}

void UChronoRiftComp::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	bShouldLaunch = false;
	bIsLockingTargetArea = false;
	LockedTargets.Empty();
	TargetAreaCenter = FVector::ZeroVector;
	
	//CurrentChronoRiftZone->Destroy();
	//CurrentChronoRiftZone = nullptr;
}

void UChronoRiftComp::Server_PerformLaunch_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}

	Multicast_PerformLaunch();
}

void UChronoRiftComp::Multicast_PerformLaunch_Implementation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (ChronoRiftZoneClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Spawning ChronoRiftZone!"), *FString(__FUNCTION__));
		CurrentChronoRiftZone = GetWorld()->SpawnActor<AChronoRiftZone>(ChronoRiftZoneClass, TargetAreaCenter, FRotator::ZeroRotator);
		
		if (CurrentChronoRiftZone)
		{
			CurrentChronoRiftZone->SetOwnerCharacter(OwnerCharacter);
			CurrentChronoRiftZone->SetInitialValues(
				GetAttackRadius(),
				GetChronoDuration(),
				GetDamageAmount()
				);
			/*UE_LOG(LogTemp, Warning, TEXT("Radius: %f, Lifetime: %f, DamageAmount: %f"),
				GetAttackRadius(), GetChronoDuration(), GetDamageAmount());*/
			LovesMesh->SetActorHiddenInGame(true);
		}
	}
}

float UChronoRiftComp::GetChronoDuration() const
{
	return ChronoDuration;
}

float UChronoRiftComp::GetAttackRadius() const
{
	if (AttackDamageModifier == 1.f)
	{
		return TargetAreaRadius;
	}
	
	return TargetAreaRadius + (AttackDamageModifier * 50.f);
}

float UChronoRiftComp::GetAttackCooldown() const
{
	return Super::GetAttackCooldown() * AttackSpeedModifier;
}

float UChronoRiftComp::GetDamageAmount() const
{
	if (FMath::IsNearlyEqual(AttackDamageModifier, 1.f, 0.0001f)) //if (AttackDamageModifier == 1.f)
	{
		return Super::GetDamageAmount();
	}
	return Super::GetDamageAmount() + AttackDamageModifier;
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
