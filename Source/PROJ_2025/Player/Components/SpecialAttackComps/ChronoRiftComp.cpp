#include "ChronoRiftComp.h"

#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "Golem.h"
#include "GameFramework/Character.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/ChronoRiftZone.h"


UChronoRiftComp::UChronoRiftComp()
{
	PrimaryComponentTick.bCanEverTick = true;

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
				const FVector SpawnLocation = FVector(TargetAreaCenter.X, TargetAreaCenter.Y, TargetAreaCenter.Z + 5.f);
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

	if (!OwnerCharacter->IsAlive())
	{
		return;
	}
	
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
	DOREPLIFETIME(UChronoRiftComp, CurrentChronoRiftZone);
	DOREPLIFETIME(UChronoRiftComp, IndicatorRadius);
}

void UChronoRiftComp::BeginPlay()
{
	Super::BeginPlay();
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
		if (HitResult.GetActor()->IsA(AGolem::StaticClass()))
		{
			if (OwnerCharacter->HasAuthority())
			{
				TargetAreaCenter = FVector::ZeroVector;
			}
			else
			{
				Server_SetTargetAreaCenter(FVector::ZeroVector);
				TargetAreaCenter = FVector::ZeroVector;
			}
			return;
		}
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
	if (!LovesMesh)
	{
		if (ChronoRiftIndicatorClass)
		{
			LovesMesh = GetWorld()->SpawnActor<AActor>(ChronoRiftIndicatorClass, TargetAreaCenter, FRotator::ZeroRotator);
			SetIndicatorHidden(true);
		}
	}
	UpdateIndicatorScale();
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
	SetIndicatorHidden(true);
	StartAttack();
}

void UChronoRiftComp::PrepareForLaunch()
{
	if (!OwnerCharacter || !Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}
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
		FActorSpawnParameters Params;
		Params.Owner = OwnerCharacter;

		if (!OwnerCharacter->GetInstigator())
		{
			UE_LOG(LogTemp, Warning, TEXT("OwnerCharacter's Instigator is NULL in %s"), *FString(__FUNCTION__));
			return;
		}
	
		Params.Instigator = OwnerCharacter->GetInstigator();
		
		CurrentChronoRiftZone = GetWorld()->SpawnActor<AChronoRiftZone>(ChronoRiftZoneClass, TargetAreaCenter, FRotator::ZeroRotator, Params);
		
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

void UChronoRiftComp::SetIndicatorHidden(const bool bIsHidden)
{
	if (!LovesMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Mesh"));
		return;
	}
	
	LovesMesh->SetActorHiddenInGame(bIsHidden);
}

void UChronoRiftComp::UpdateIndicatorScale()
{
	if (!LovesMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s, No Mesh"), *FString(__FUNCTION__));
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%s, Updating Indicator Scale"), *FString(__FUNCTION__));

	if (UStaticMeshComponent* MeshComp = LovesMesh->FindComponentByClass<UStaticMeshComponent>())
	{
		if (!MeshComp->IsRegistered())
		{
			MeshComp->RegisterComponent();
		}
		
		FVector AssetExtent = FVector::ZeroVector;
		if (MeshComp->GetStaticMesh())
		{
			AssetExtent = MeshComp->GetStaticMesh()->GetBounds().BoxExtent;
		}

		float CurrentWorldDiameterXY;

		if (!AssetExtent.IsNearlyZero())
		{
			const FVector CompScale = MeshComp->GetComponentScale();
			const float WorldExtentX = AssetExtent.X * CompScale.X;
			const float WorldExtentY = AssetExtent.Y * CompScale.Y;
			CurrentWorldDiameterXY = FMath::Max(WorldExtentX, WorldExtentY) * 2.0f;
		}
		else
		{
			CurrentWorldDiameterXY = FMath::Max(MeshComp->Bounds.BoxExtent.X, MeshComp->Bounds.BoxExtent.Y) * 2.0f;
		}

		if (CurrentWorldDiameterXY > KINDA_SMALL_NUMBER)
		{
			GetAttackRadius();
			const float NewIndicatorRadius = IndicatorRadius * 2.0f;
			const float DesiredDiameter = NewIndicatorRadius;
			const float ScaleFactor = DesiredDiameter / CurrentWorldDiameterXY;
			const FVector NewScale = MeshComp->GetComponentScale() * ScaleFactor;
			MeshComp->SetWorldScale3D(FVector(NewScale.X, NewScale.Y, 1.0f));
		}
	}
}

float UChronoRiftComp::GetChronoDuration() const
{
	return ChronoDuration;
}

float UChronoRiftComp::GetAttackRadius()
{
	if (FMath::IsNearlyEqual(AttackDamageModifier, 1.f, 0.0001f))
	{
		return TargetAreaRadius;
	}
	float NewRadius = TargetAreaRadius + (AttackDamageModifier * 50.f);
	Server_SetIndicatorRadius(NewRadius);
	return NewRadius;
}

float UChronoRiftComp::GetAttackCooldown() const
{
	return Super::GetAttackCooldown() * AttackSpeedModifier;
}

float UChronoRiftComp::GetDamageAmount() const
{
	if (FMath::IsNearlyEqual(AttackDamageModifier, 1.f, 0.0001f))
	{
		return Super::GetDamageAmount();
	}
	return Super::GetDamageAmount() + AttackDamageModifier;
}

void UChronoRiftComp::Server_SetIndicatorRadius_Implementation(const float NewRadius)
{
	IndicatorRadius = NewRadius;
}

void UChronoRiftComp::Server_SetTargetAreaCenter_Implementation(const FVector& TargetCenter)
{
	if (TargetCenter.IsNearlyZero())
	{
		TargetAreaCenter = FVector::ZeroVector;
		return;
	}
	
	TargetAreaCenter = TargetCenter;
}
