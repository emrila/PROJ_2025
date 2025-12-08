#include "DashAttackComp.h"

#include "EnemyBase.h"
#include "EnhancedInputComponent.h"
#include "KismetTraceUtils.h"
#include "ShadowStrikeVariant2.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/ShadowStrikeRibbon.h"
#include "Player/Components/Items/Shield.h"


UDashAttackComp::UDashAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	DamageAmount = 20.f;
	AttackCooldown = 4.f;
}

void UDashAttackComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (bHasLockedTargetLocation && bCanAttack)
	{
		TryLockingTargetLocation();
		
		if (!TargetLocation.IsNearlyZero())
		{
#if WITH_EDITOR
	if (bDrawDebug)
	{
		DrawDebugSphere(GetWorld(), TargetLocation, 50.f, 10, FColor::Green, false, 0.1f);
	}
#endif
		}
	}
	
	
	if (bIsDashing)
	{
		DashElapsed += DeltaTime;
		const float DashAlpha = FMath::Clamp(DashElapsed / DashDuration, 0.0f, 1.0f);
		const FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, DashAlpha);
		OwnerCharacter->SetActorLocation(NewLocation, false);
		
		if (DashAlpha >= 1.0f)
		{
			bIsDashing = false;
			HandlePostAttackState();
		}
	}

}

void UDashAttackComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &UDashAttackComp::OnPrepareForAttack);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this, &UDashAttackComp::OnStartAttack);
	}
}

void UDashAttackComp::StartAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (!OwnerCharacter->IsAlive())
	{
		return;
	}

	if (!bCanAttack)
	{
		if (!bShouldRecast)
		{
			return;
		}
	}
	
	TryLockingTargetLocation();
	
	if (TargetLocation.IsNearlyZero())
	{
		return;
	}
	
	if (bShouldRecast)
	{
		GetWorld()->GetTimerManager().ClearTimer(RecastTimer);
		Server_SetShouldRecast(false);
		Server_SetDidRecast(true);
	}
	
	PerformAttack();
	Super::StartAttack();
}


void UDashAttackComp::BeginPlay()
{
	Super::BeginPlay();
	
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (!Ribbon)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = OwnerCharacter;
		SpawnParams.Instigator = OwnerCharacter->GetInstigator();
		Ribbon = Cast<AShadowStrikeRibbon>(
			GetWorld()->SpawnActor<AShadowStrikeRibbon>(RibbonClass, SpawnParams));
		
		if (!Ribbon)
		{
			UE_LOG(ShadowStrikeLog, Error, TEXT("%s Ribbon is Null after spawning."), *FString(__FUNCTION__));
		}
	}
}

void UDashAttackComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDashAttackComp, Ribbon);
	
	DOREPLIFETIME(UDashAttackComp, TargetLocation);
	DOREPLIFETIME(UDashAttackComp, StartLocation);
	
	DOREPLIFETIME(UDashAttackComp, bShouldRecast);
	DOREPLIFETIME(UDashAttackComp, bDidRecast);
	DOREPLIFETIME(UDashAttackComp, bWentThroughShield);
	DOREPLIFETIME(UDashAttackComp, bCanDash);
	DOREPLIFETIME(UDashAttackComp, bIsDashing);
	DOREPLIFETIME(UDashAttackComp, bHasLockedTargetLocation);
}

void UDashAttackComp::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	if (TargetLocation.IsNearlyZero())
	{
		UE_LOG(LogTemp, Error, TEXT("%s TargetLocation is Zero."), *FString(__FUNCTION__));
		return;
	}
	
	Dash();
	Server_PerformSweep();
	
	//handle post attack
}

void UDashAttackComp::OnPrepareForAttack(const FInputActionInstance& ActionInstance)
{
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}

	bHasLockedTargetLocation = true;
	PrepareForAttack();
}

void UDashAttackComp::OnStartAttack(const FInputActionInstance& ActionInstance)
{
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}

	if (!bHasLockedTargetLocation)
	{
		return;
	}

	if (OwnerCharacter->HasAuthority())
	{
		TargetLocation = FVector::ZeroVector;
		StartLocation = FVector::ZeroVector;
	}
	else
	{
		Server_SetStartAndTargetLocation(FVector::ZeroVector, FVector::ZeroVector);
		TargetLocation = FVector::ZeroVector;
		StartLocation = FVector::ZeroVector;
	}

	bHasLockedTargetLocation = false;
	StartAttack();
}

void UDashAttackComp::HandlePostAttackState()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (OwnerCharacter->GetCapsuleComponent() && OwnerCharacter->GetMesh())
	{
		//OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		FTimerHandle MeshvisibilityTimer;
		GetWorld()->GetTimerManager().SetTimer(MeshvisibilityTimer, [this]()
		{
			OwnerCharacter->GetMesh()->SetVisibility(true, true);
		}, 0.1f, false);
		
	}
}

void UDashAttackComp::PrepareForAttack()
{
	//TODO: Handle before attack animation here 
}

void UDashAttackComp::TryLockingTargetLocation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}

	if (!OwnerCharacter->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s Pawn is not locally controlled; skipping camera-based locking."),
			   *FString(__FUNCTION__));
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
	
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	FVector PlayerForward = OwnerCharacter->GetActorForwardVector();
	
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = CameraLocation + (CameraRotation.Vector() * DashRange);
	
	FVector NewEndLocation = TraceEnd;
	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	/*const bool bHit = GetWorld()->SweepSingleByObjectType(
		HitResult,
		TraceStart,
		NewEndLocation,
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(50.f),
		Params
	);*/
	
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		TraceStart,
		NewEndLocation,
		ObjectQueryParams,
		Params
	);
	
#if WITH_EDITOR
	if (bDrawDebug)
	{
		//DrawDebugSweptSphere(GetWorld(), TraceStart, TraceEnd, 50.f, FColor::Purple, false, 5.f);
	}
#endif
	
	if (bHit && HitResult.GetActor())
	{
		FVector TraceDir = (TraceEnd - TraceStart).GetSafeNormal();
		if (TraceDir.IsNearlyZero()) { TraceDir = CameraRotation.Vector(); }
		
		const FVector ToImpact = HitResult.ImpactPoint - PlayerLocation;
		const float ForwardDot = FVector::DotProduct(ToImpact.GetSafeNormal(), PlayerForward);

		// Accept impact only if it's in front of the player. Small threshold avoids borderline cases.
		const float ForwardThreshold = 0.1f;
		if (ForwardDot > ForwardThreshold)
		{
			NewEndLocation = HitResult.ImpactPoint - TraceDir * 20.f; // back off a bit
		}
		// else: impact is behind the player (even if in front of camera) -> ignore and keep TraceEnd
	}

	if (OwnerCharacter->HasAuthority())
	{
		TargetLocation = NewEndLocation;
		StartLocation = PlayerLocation;
	}
	else
	{
		Server_SetStartAndTargetLocation(PlayerLocation, NewEndLocation);
		TargetLocation = NewEndLocation;
		StartLocation = PlayerLocation;
	}
	
	if (!TargetLocation.IsNearlyZero())
	{
		Server_SetCanDash(true);
	}
}

void UDashAttackComp::Server_SetIsDashing_Implementation(const bool bNewIsDashing)
{
	bIsDashing = bNewIsDashing;
}

void UDashAttackComp::Server_SetHasLockedTargetLocation_Implementation(const bool bNewHasLockedTargetLocation)
{
	bHasLockedTargetLocation = bNewHasLockedTargetLocation;
}

void UDashAttackComp::Dash()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (OwnerCharacter->HasAuthority())
	{
		if (bIsDashing) { return; }
	
		DashElapsed = 0.0f;
	
		if (OwnerCharacter->GetCapsuleComponent() && OwnerCharacter->GetMesh())
		{
			//OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
			OwnerCharacter->GetMesh()->SetVisibility(false, true);
		}
	
		bIsDashing = true;
	}
	else
	{
		Server_Dash();
		/*if (bIsDashing) { return; }
	
		DashElapsed = 0.0f;
	
		if (OwnerCharacter->GetCapsuleComponent())
		{
			OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		}
	
		bIsDashing = true;*/
	}
}

void UDashAttackComp::Server_Dash_Implementation()
{
	if (!OwnerCharacter) 
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	if (bIsDashing) { return; }
	
	DashElapsed = 0.0f;
	
	if (OwnerCharacter->GetCapsuleComponent())
	{
		//OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		OwnerCharacter->GetMesh()->SetVisibility(false, true);
	}
	
	bIsDashing = true;
}

void UDashAttackComp::Server_SetCanDash_Implementation(const bool Value)
{
	bCanDash = Value;
}

void UDashAttackComp::Server_SetWentThroughShield_Implementation(const bool Value)
{
	bWentThroughShield = Value;
}

void UDashAttackComp::Server_SetStartAndTargetLocation_Implementation(const FVector& NewStartLocation,
                                                                      const FVector& NewLockedLocation)
{
	StartLocation = NewStartLocation;
	TargetLocation = NewLockedLocation;
}

void UDashAttackComp::Server_SetShouldRecast_Implementation(const bool bNewShouldRecast)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	bShouldRecast = bNewShouldRecast;
	if (bShouldRecast)
	{
		OnCanRecast.Broadcast();
		OwnerCharacter->StartIFrame();
		GetWorld()->GetTimerManager().SetTimer(IFrameTimer, [this]()
		{
			OwnerCharacter->ResetIFrame();
		}, RecastDuration, false);
	}
}

void UDashAttackComp::Server_SetDidRecast_Implementation(const bool bNewDidRecast)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	bDidRecast = bNewDidRecast;
	if (bDidRecast)
	{
		OnRecast.Broadcast();
	}
}

void UDashAttackComp::Server_PerformSweep_Implementation()
{
	if (TargetLocation.IsNearlyZero())
	{
		return;
	}

	if (!OwnerCharacter)
	{
		return;
	}

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

	if (GetWorld())
	{
		bool bHit = GetWorld()->SweepMultiByObjectType(
			HitResults,
			StartLocation,
			TargetLocation,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(50.f),
			Params
		);
		
#if WITH_EDITOR
		if (bDrawDebug)
		{
			DrawDebugSweptSphere(GetWorld(), StartLocation, TargetLocation, 50.f, FColor::Red, false, 5.f);
		}
#endif
		
		if (Ribbon)
		{
			Ribbon->SetActorLocation(StartLocation);
			Ribbon->BP_SpawnRibbon(StartLocation, TargetLocation, DashDuration);
		}
		
		TSet<AActor*> Enemies;
		if (bHit)
		{
			for (const FHitResult& Hit : HitResults)
			{
				if (AActor* HitActor = Hit.GetActor())
				{
					if (HitActor->IsA(AEnemyBase::StaticClass()))
					{
						Enemies.Add(HitActor);
					}
					if (HitActor->IsA(AShield::StaticClass()))
					{
						Server_SetWentThroughShield(true);
					}
				}
			}
		}

		if (Enemies.Num() > 0)
		{
			for (AActor* EnemyActor : Enemies)
			{
				if (EnemyActor)
				{
					UGameplayStatics::ApplyDamage(
						EnemyActor,
						GetDamageAmount(),
						OwnerCharacter->GetController(),
						OwnerCharacter,
						UDamageType::StaticClass()
					);
				}
			}
			if (bDidRecast) { return;}
			Server_SetShouldRecast(true);
			GetWorld()->GetTimerManager().SetTimer(RecastTimer, [this] ()
			{
				Server_SetShouldRecast(false);
			}, RecastDuration, false);
		}
		
	}
}

void UDashAttackComp::ResetAttackCooldown()
{
	Super::ResetAttackCooldown();
	Server_SetStartAndTargetLocation_Implementation(FVector::ZeroVector, FVector::ZeroVector);
	Server_SetShouldRecast(false);
	Server_SetDidRecast(false);
	Server_SetWentThroughShield(false);
	Server_SetCanDash(false);
	Server_SetIsDashing(false);
	Server_SetHasLockedTargetLocation(false);
	bShouldRecast = false;
	bDidRecast = false;
	bWentThroughShield = false;
	bCanDash = false;
	bIsDashing = false;
	bHasLockedTargetLocation = false;
}

float UDashAttackComp::GetAttackCooldown() const
{
	if (FMath::IsNearlyEqual((Super::GetAttackCooldown() * AttackSpeedModifier), 0.5f))
	{
		return 0.5f;
	}
	
	return Super::GetAttackCooldown() * AttackSpeedModifier;
}

float UDashAttackComp::GetDamageAmount() const
{
	if (FMath::IsNearlyEqual(AttackDamageModifier, 1.f, 0.0001f))
	{
		return Super::GetDamageAmount();
	}
	return Super::GetDamageAmount() + (AttackDamageModifier * 20.f);
}
