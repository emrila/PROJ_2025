#include "DashAttackComp.h"

#include "EnemyBase.h"
#include "EnhancedInputComponent.h"
#include "KismetTraceUtils.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/ShadowStrikeRibbon.h"
#include "Player/Components/Items/Shield.h"

UDashAttackComp::UDashAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	DamageAmount = 25.f;
	AttackCooldown = 15.f;
}

void UDashAttackComp::TickComponent(float DeltaTime, ELevelTick TickType,
										 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
#if WITH_EDITOR
	if (bIsLockingTargetLocation)
	{
		if (const FVector TargetLoc = GetTargetLocation(); !TargetLoc.IsNearlyZero())
		{
			DrawDebugSphere(GetWorld(), TargetLoc, 50.f, 10, FColor::Green, false, 0.1f);
		}
	}
#endif
	
	if (!bIsDashing || !OwnerCharacter)
	{
		return;
	}
	
	DashElapsed += DeltaTime;
	const float DashAlpha = FMath::Clamp(DashElapsed / DashDuration, 0.0f, 1.0f);
	const FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, DashAlpha);
	if (OwnerCharacter->GetCharacterMovement())
	{
		OwnerCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}
	
	FHitResult HitResult;
	OwnerCharacter->SetActorLocation(NewLocation, true, &HitResult);
	
	if (HitResult.bBlockingHit || DashElapsed >= DashDuration)
	{
		bIsDashing = false;
		HandlePostAttackState();
	}
}

void UDashAttackComp::BeginPlay()
{
	Super::BeginPlay();
	
	if (RibbonClass)
	{
		Ribbon = GetWorld()->SpawnActor<AShadowStrikeRibbon>(RibbonClass, FVector::ZeroVector, FRotator::ZeroRotator);
	}
}

void UDashAttackComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UDashAttackComp, DashElapsed);
	DOREPLIFETIME(UDashAttackComp, bIsDashing);
	DOREPLIFETIME(UDashAttackComp, bShouldRecast);
	DOREPLIFETIME(UDashAttackComp, bDidRecast);
	DOREPLIFETIME(UDashAttackComp, StartLocation);
	DOREPLIFETIME(UDashAttackComp, TargetLocation);
}

void UDashAttackComp::OnPreAttack(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}
	
	if ((!bCanAttack && !bShouldRecast) || bIsDashing)
	{
		return;
	}
	
	if (OwnerCharacter)
	{
		OwnerCharacter->RequestSetIsAttacking(true);
	}
	bIsLockingTargetLocation = true;
}

void UDashAttackComp::OnStartAttack(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}
	if (!bIsLockingTargetLocation)
	{
		return;
	}
	
	if ((!bCanAttack && !bShouldRecast) || bIsDashing)
	{
		return;
	}
	
	if (OwnerCharacter)
	{
		OwnerCharacter->RequestSetIsAttacking(false);
	}
	
	bIsLockingTargetLocation = false;
	StartAttack();
}

void UDashAttackComp::StartAttack()
{
	if (!OwnerCharacter || !OwnerCharacter->IsAlive())
	{
		return;
	}
	
	if (!bCanAttack && !bShouldRecast)
	{
		return;
	}
	
	if (bShouldRecast)
	{
		GetWorld()->GetTimerManager().ClearTimer(RecastTimer);
		SetShouldRecast(false);
		SetDidRecast(true);
		OnRecast.Broadcast();
	}
	
	PerformAttack();
	Super::StartAttack();
}

void UDashAttackComp::PerformAttack()
{
	const FVector NewTargetLocation = GetTargetLocation();
	if (NewTargetLocation.IsNearlyZero())
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s NewTargetLocation is Zero."), *FString(__FUNCTION__));
		return;
	}
	
	RequestDash(NewTargetLocation);
}

void UDashAttackComp::RequestDash(const FVector& NewTargetLocation)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	if (OwnerCharacter->HasAuthority())
	{
		Dash(NewTargetLocation);
	}
	else
	{
		Server_Dash(NewTargetLocation);
	}
}

void UDashAttackComp::Dash(const FVector& NewTargetLocation)
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	if (bIsDashing)
	{
		return;
	}
	TargetLocation = NewTargetLocation;
	StartLocation = OwnerCharacter->GetActorLocation();
	
	// Perform the sweep and spawn the effect here 
	PerformSweep();
	
	
	
	//Disable input, start iframes, set elapsed to 0 while dashing, and handle collison
	if (OwnerCharacter->GetCapsuleComponent())
	{
		OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	OwnerCharacter->SetInputActive(false);
	OwnerCharacter->StartIFrame();
	DashElapsed = 0.0f;
	
	bIsDashing = true;
}

void UDashAttackComp::Server_Dash_Implementation(const FVector& NewTargetLocation)
{
	Dash(NewTargetLocation);
}

void UDashAttackComp::PerformSweep()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	if (Ribbon)
	{
		Ribbon->SetActorLocation(StartLocation);
		Ribbon->SetActorRotation(OwnerCharacter->GetActorRotation());
		Ribbon->BP_SpawnRibbon(StartLocation, TargetLocation, DashDuration);
	}

	TArray<FHitResult> HitResults;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_GameTraceChannel3);

	if (GetWorld())
	{
		
		const float Radius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
		const float HalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		bool bHit = GetWorld()->SweepMultiByObjectType(
			HitResults,StartLocation,TargetLocation,FQuat::Identity,ObjectQueryParams,FCollisionShape::MakeCapsule(Radius, HalfHeight));
		
#if WITH_EDITOR
		DrawDebugSweptSphere(GetWorld(), StartLocation, TargetLocation, 50.f, FColor::Red, false, 5.f);
#endif
		
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
						bWentThroughShield = true;
						OnRepWentThroughShield();
						bWentThroughShield = false;
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
					UGameplayStatics::ApplyDamage(EnemyActor, GetDamageAmount(), OwnerCharacter->GetController(), OwnerCharacter, UDamageType::StaticClass());
				}
			}
			if (bShouldEverRecast)
			{
				if (bDidRecast)
				{
					bDidRecast = false;
					return;
				}
				bShouldRecast = true;
				OnRepShouldRecast();
			}
		}
		
	}
}

float UDashAttackComp::GetCooldownDuration()
{
	if (Super::GetCooldownDuration() <= RecastDuration)
	{
		SetShouldEverRecast(false);
	}
	else
	{
		SetShouldEverRecast(true);
	}
	return Super::GetCooldownDuration();
}

void UDashAttackComp::SetShouldEverRecast(const bool bNewShouldEverRecast)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	bShouldEverRecast = bNewShouldEverRecast;
	if (!OwnerCharacter->HasAuthority())
	{
		Server_SetShouldEverRecast(bNewShouldEverRecast);
	}
}

void UDashAttackComp::SetShouldRecast(const bool bNewShouldRecast)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	bShouldRecast = bNewShouldRecast;
	if (!OwnerCharacter->HasAuthority())
	{
		Server_SetShouldRecast(bNewShouldRecast);
	}
}

void UDashAttackComp::Server_SetShouldRecast_Implementation(const bool bNewShouldRecast)
{
	bShouldRecast = bNewShouldRecast;
}

void UDashAttackComp::SetDidRecast(const bool bNewDidRecast)
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	bDidRecast = bNewDidRecast;
	if (!OwnerCharacter->HasAuthority())
	{
		Server_SetDidRecast(bNewDidRecast);
	}
}

void UDashAttackComp::Server_SetDidRecast_Implementation(const bool bNewDidRecast)
{
	bDidRecast = bNewDidRecast;
}

void UDashAttackComp::Server_SetShouldEverRecast_Implementation(const bool bNewShouldEverRecast)
{
	bShouldEverRecast = bNewShouldEverRecast;
}

void UDashAttackComp::Server_SetIsDashing_Implementation(const bool bNewIsDashing)
{
	bIsDashing = bNewIsDashing;
}

FVector UDashAttackComp::GetTargetLocation() const
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return FVector::ZeroVector;
	}

	if (!OwnerCharacter->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s Pawn is not locally controlled; skipping camera-based locking."),
			   *FString(__FUNCTION__));
		return FVector::ZeroVector;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerController is Null."), *FString(__FUNCTION__));
		return FVector::ZeroVector;
	}

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;

	if (!CameraManager)
	{
		UE_LOG(LogTemp, Error, TEXT("%s CameraManager is Null."), *FString(__FUNCTION__));
		return FVector::ZeroVector;
	}
	
	FVector CameraLocation = CameraManager->GetCameraLocation();
	FRotator CameraRotation = CameraManager->GetCameraRotation();
	
	FVector PlayerLocation = OwnerCharacter->GetActorLocation();
	
	float NewDashRange = (CameraLocation - PlayerLocation).Size() + DashRange;
	FVector NewTargetLocation = CameraLocation + (CameraRotation.Vector() * NewDashRange);
	
	// Use a line trace to find the first blocking hit in case of getting a location for the UI indicator
	/*FHitResult HitResult;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		HitResult,
		PlayerLocation,
		TraceEnd,
		ObjectQueryParams
	);
	
	if (bHit) { TraceEnd = HitResult.ImpactPoint; }*/
	
	// Ensure the dash target location is above the ground
	// 20.f is an offset value to prevent dashing into the ground
	if (NewTargetLocation.Z < PlayerLocation.Z + 20.f)
	{
		NewTargetLocation.Z = PlayerLocation.Z + 5.f;
	}
	
	return NewTargetLocation;
}

void UDashAttackComp::HandlePostAttackState() const
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return;
	}
	
	OwnerCharacter->SetInputActive(true);
	if (OwnerCharacter->GetCapsuleComponent())
	{
		OwnerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}
	// Check whether the iframe timer has been started by going through a shield
	if (!GetWorld()->GetTimerManager().IsTimerActive(IFrameTimer))
	{
		OwnerCharacter->ResetIFrame();
	}
}

void UDashAttackComp::Reset()
{
	Super::Reset();
	SetDidRecast(false);
	SetShouldRecast(false);
}

void UDashAttackComp::OnRepShouldRecast()
{
	if (bShouldRecast)
	{
		GetWorld()->GetTimerManager().SetTimer(RecastTimer, [this] ()
		{
			bShouldRecast = false;
		}, RecastDuration, false);
		OnCanRecast.Broadcast();
	}
}

void UDashAttackComp::OnRepWentThroughShield()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (bWentThroughShield)
	{
		OwnerCharacter->StartIFrameVisuals();
		if (GetWorld()->GetTimerManager().IsTimerActive(IFrameTimer))
		{
			GetWorld()->GetTimerManager().ClearTimer(IFrameTimer);
		}
		GetWorld()->GetTimerManager().SetTimer(IFrameTimer, [this] ()
			{
				OwnerCharacter->ResetIFrameVisuals();
			}, ShieldInvincibilityDuration, false);
	}
}