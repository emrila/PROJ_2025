#include "ChronoRiftComp.h"

#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/ChronoRiftZone.h"


UChronoRiftComp::UChronoRiftComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);

	DamageAmount = 2.f;
	AttackCooldown = 3.f;
	ChronoDuration = 100.f;

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

	SetIndicatorHidden(true);
	
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
	
	DOREPLIFETIME(UChronoRiftComp, TargetAreaCenter);
	DOREPLIFETIME(UChronoRiftComp, LovesMesh);
	DOREPLIFETIME(UChronoRiftComp, CurrentChronoRiftZone);
}

void UChronoRiftComp::BeginPlay()
{
	Super::BeginPlay();
	
	if (ChronoRiftIndicatorClass)
	{
		LovesMesh = GetWorld()->SpawnActor<AActor>(ChronoRiftIndicatorClass, TargetAreaCenter, FRotator::ZeroRotator);
		
		if (LovesMesh)
		{
			InitialIndicatorScale = LovesMesh->GetActorScale3D();
			if (OwnerCharacter && OwnerCharacter->HasAuthority())
			{
				SetIndicatorHidden(true);
			}
			
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

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	//Line trace to find the ground location
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult, 
		TraceStart, 
		TraceEnd, 
		ObjectQueryParams
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
	}
	
}

void UChronoRiftComp::OnStartLockingTargetArea(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}

	if (!bCanAttack)
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
	if (!OwnerCharacter || !Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}
	//Handle animation and spawning the circle here
	SetIndicatorHidden(false);
}

void UChronoRiftComp::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	bIsLockingTargetArea = false;
	TargetAreaCenter = FVector::ZeroVector;
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
		CurrentChronoRiftZone = GetWorld()->SpawnActor<AChronoRiftZone>(ChronoRiftZoneClass, TargetAreaCenter, FRotator::ZeroRotator);
		
		if (CurrentChronoRiftZone)
		{
			CurrentChronoRiftZone->SetOwnerCharacter(OwnerCharacter);
			CurrentChronoRiftZone->SetInitialValues(
				GetAttackRadius(),
				GetChronoDuration(),
				GetDamageAmount()
				);
		}
	}
}

void UChronoRiftComp::SetIndicatorHidden(bool bIsHidden)
{
	if (!LovesMesh)
	{
		return;
	}
	LovesMesh->SetActorHiddenInGame(bIsHidden);
	
	if (!bIsHidden)
	{
		FVector SpawnLocation = FVector(TargetAreaCenter.X, TargetAreaCenter.Y, TargetAreaCenter.Z + 5.f);
		LovesMesh->SetActorLocation(SpawnLocation);
		
		const float DesiredDiameter = GetAttackRadius() * 2.0f;

		if (UStaticMeshComponent* MeshComp = LovesMesh->FindComponentByClass<UStaticMeshComponent>())
		{
			const float CurrentWorldDiameterXY = FMath::Max(MeshComp->Bounds.BoxExtent.X, MeshComp->Bounds.BoxExtent.Y) * 2.0f;

			if (CurrentWorldDiameterXY > KINDA_SMALL_NUMBER)
			{
				const float ScaleFactor = DesiredDiameter / CurrentWorldDiameterXY;
				const FVector NewScale = MeshComp->GetComponentScale() * ScaleFactor;
				MeshComp->SetWorldScale3D(NewScale);

				// Keep Z scale small so plane stays flat
				MeshComp->SetWorldScale3D(FVector(NewScale.X, NewScale.Y, 1.0f));
			}
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

void UChronoRiftComp::Server_SetTargetAreaCenter_Implementation(const FVector& TargetCenter)
{
	if (TargetCenter.IsNearlyZero())
	{
		UE_LOG(LogTemp, Error, TEXT("%s TargetCenter is Zero."), *FString(__FUNCTION__));
		TargetAreaCenter = TargetCenter;
		return;
	}
	
	TargetAreaCenter = TargetCenter;
}
