#include "MeleeAttackComp.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"


UMeleeAttackComp::UMeleeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	DamageAmount = 30.0f;
}

void UMeleeAttackComp::StartAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	if (!bCanAttack || !OwnerCharacter->IsAlive())
	{
		return;
	}
	
	if (OwnerCharacter->IsAttacking())
	{
		return;
	}
	
	Super::StartAttack();
	PerformAttack();
}

void UMeleeAttackComp::PerformAttack()
{
	Super::PerformAttack();
	RequestSweep();
}

void UMeleeAttackComp::RequestSweep()
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	const FVector SweepLocation =OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * AttackSweepRadius;
	if (OwnerCharacter->HasAuthority())
	{
		Sweep(SweepLocation);
	}
	else
	{
		Server_Sweep(SweepLocation);
	}
}

void UMeleeAttackComp::Sweep(const FVector& SweepLocation)
{
	TArray<FHitResult> HitResults;

	FCollisionQueryParams QueryParams;
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	
	const bool bHit = GetWorld()->SweepMultiByObjectType(
		HitResults,
		SweepLocation,
		SweepLocation, 
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(AttackSweepRadius),
		QueryParams);
#if WITH_EDITOR
	if (bDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			SweepLocation,
			AttackSweepRadius,
			12,
			bHit ? FColor::Red : FColor::Green,
			false,
			2.0f
			);
	}
#endif
	if (bHit)
	{
		TSet<AActor*> UniqueHitActors;
		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.GetActor())
			{
				UniqueHitActors.Add(Hit.GetActor());
				Multicast_SpawnImpactParticles(Hit.ImpactPoint);
			}
		}
		if (OwnerCharacter && OwnerCharacter->GetController())
		{
			for (AActor* Actor : UniqueHitActors)
			{
				UGameplayStatics::ApplyDamage(
					Actor,
					GetDamageAmount(),
					OwnerCharacter->GetController(),
					OwnerCharacter,
					UDamageType::StaticClass()
					);
			}
		}
	}
}

void UMeleeAttackComp::Server_Sweep_Implementation(const FVector& SweepLocation)
{
	Sweep(SweepLocation);
}

