#include "ChronoRiftComp.h"

#include "InputAction.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/ChronoRiftZone.h"

UChronoRiftComp::UChronoRiftComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	DamageAmount = 2.f;
	AttackCooldown = 15.f;
}


void UChronoRiftComp::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerHandle TimerHandle; 
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		if (!OwnerCharacter)
		{
			UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
			return;
		}
		if (IndicatorActorClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = OwnerCharacter;
			SpawnParams.Instigator = OwnerCharacter;
			IndicatorActor = GetWorld()->SpawnActor<AActor>(
				IndicatorActorClass,
				FTransform(FRotator::ZeroRotator, FVector::ZeroVector),
				SpawnParams);
			SetIndicatorHidden(true);
		}
	}, 1.5f, false);
}

void UChronoRiftComp::OnPreAttack(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Started || !bCanAttack)
	{
		return;
	}
	if (OwnerCharacter)
	{
		OwnerCharacter->RequestSetIsAttacking(true);
	}
	Super::OnPreAttack(InputActionInstance);
	SetIndicatorHidden(false);
}

void UChronoRiftComp::OnStartAttack(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Completed || !bCanAttack)
	{
		return;
	}
	
	if (OwnerCharacter)
	{
		OwnerCharacter->RequestSetIsAttacking(false);
	}
	
	Super::OnStartAttack(InputActionInstance);
	Super::StartAttack();
	
	PerformAttack();
}

void UChronoRiftComp::StartAttack()
{
	if (!bCanAttack || !OwnerCharacter || !OwnerCharacter->IsAlive() || !IndicatorActor)
	{
		return;
	}
	
	if (IndicatorActor)
	{
		FVector IndicatorLocation = GetIndicatorActorLocation();
		IndicatorLocation.Z += 5.f;
		IndicatorActor->SetActorLocation(IndicatorLocation);
	}
}

void UChronoRiftComp::PerformAttack()
{
	SetIndicatorHidden(true);
	RequestSpawnChronoRiftZone();
}

void UChronoRiftComp::UpdateIndicatorScale() const
{
	if (!IndicatorActor)
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("%s, No Indicator Actor"), *FString(__FUNCTION__));
		return;
	}
	
	if (UStaticMeshComponent* MeshComp = IndicatorActor->FindComponentByClass<UStaticMeshComponent>())
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
			const float IndicatorRadius = GetAttackRadius();
			const float NewIndicatorRadius = IndicatorRadius * 2.0f;
			const float DesiredDiameter = NewIndicatorRadius;
			const float ScaleFactor = DesiredDiameter / CurrentWorldDiameterXY;
			const FVector NewScale = MeshComp->GetComponentScale() * ScaleFactor;
			MeshComp->SetWorldScale3D(FVector(NewScale.X, NewScale.Y, 1.0f));
		}
	}
}

void UChronoRiftComp::SetIndicatorHidden(const bool Value) const
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	if (!IndicatorActor)
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("%s, No Indicator Actor"), *FString(__FUNCTION__));
		return;
	}
	
	if (!OwnerCharacter->IsAlive())
	{
		return;
	}
	
	IndicatorActor->SetActorHiddenInGame(Value);
	UpdateIndicatorScale();
}

void UChronoRiftComp::RequestSpawnChronoRiftZone()
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (!OwnerCharacter->IsAlive())
	{
		return;
	}
	
	const FVector SpawnLocation = GetIndicatorActorLocation();
	
	if (OwnerCharacter->HasAuthority())
	{
		SpawnChronoRiftZone(SpawnLocation);
	}
	else
	{
		Server_SpawnChronoRiftZone(SpawnLocation);
	}
}

void UChronoRiftComp::SpawnChronoRiftZone(const FVector& SpawnLocation) const
{
	if (ChronoRiftZoneClass && OwnerCharacter)
	{
		FActorSpawnParameters Params;
		Params.Owner = OwnerCharacter;
	
		Params.Instigator = OwnerCharacter;
		
		const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
		
		AChronoRiftZone* CurrentChronoRiftZone = 
			GetWorld()->SpawnActorDeferred<AChronoRiftZone>(
				ChronoRiftZoneClass, 
				SpawnTransform, 
				OwnerCharacter, 
				OwnerCharacter, 
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
		
		if (CurrentChronoRiftZone)
		{
			CurrentChronoRiftZone->SetOwnerCharacter(OwnerCharacter);
			CurrentChronoRiftZone->SetInitialValues(
				GetAttackRadius(),
				ChronoDuration,
				GetDamageAmount()
				);
			UGameplayStatics::FinishSpawningActor(CurrentChronoRiftZone, SpawnTransform);
		}
	}
}

void UChronoRiftComp::Server_SpawnChronoRiftZone_Implementation(const FVector& SpawnLocation)
{
	SpawnChronoRiftZone(SpawnLocation);
}

FVector UChronoRiftComp::GetIndicatorActorLocation() const
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return FVector::ZeroVector;
	}

	if (!OwnerCharacter->IsLocallyControlled())
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("%s Pawn is not locally controlled; skipping camera-based locking."),
			   *FString(__FUNCTION__));
		return FVector::ZeroVector;
	}
	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());

	if (!PC)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s PlayerController is Null."), *FString(__FUNCTION__));
		return FVector::ZeroVector;
	}

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;

	if (!CameraManager)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s CameraManager is Null."), *FString(__FUNCTION__));
		return FVector::ZeroVector;
	}
	FVector CameraLocation = CameraManager->GetCameraLocation();
	FRotator CameraRotation = CameraManager->GetCameraRotation();

	FVector TraceStart = CameraLocation;
	FVector TraceEnd = TraceStart + (CameraRotation.Vector() * 3000.f);  //Replace 3000.f with the desired range
	
	FHitResult HitResult;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult, 
		TraceStart, 
		TraceEnd, 
		ObjectQueryParams
		);
	
	if (bHit && HitResult.GetActor())
	{
		return HitResult.ImpactPoint;
	}
	return FVector::ZeroVector;
}

float UChronoRiftComp::GetAttackRadius() const
{
	if (FMath::IsNearlyEqual(AttackDamageModifier, 1.f, 0.0001f))
	{
		return TargetAreaRadius;
	}
	const float NewRadius = TargetAreaRadius + (AttackDamageModifier * 50.f);
	return NewRadius;
}

void UChronoRiftComp::Debug()
{
	Super::Debug();
	UE_LOG(AttackComponentLog, Warning, TEXT("Current radius: %f"), GetAttackRadius());
	UE_LOG(AttackComponentLog, Warning, TEXT("Current duration: %f"), ChronoDuration);
}
