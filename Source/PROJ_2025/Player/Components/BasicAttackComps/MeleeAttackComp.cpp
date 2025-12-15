#include "MeleeAttackComp.h"

#include "EnemyBase.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"

UMeleeAttackComp::UMeleeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmount = 30.0f;
	AttackCooldown = 1.0f;

	// ...
}

void UMeleeAttackComp::BeginPlay()
{
	Super::BeginPlay();

	bIsFirstAttackAnimSet = false;
}

void UMeleeAttackComp::StartAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	
	if (!bCanAttack || !OwnerCharacter->IsAlive())
	{
		return;
	}

	SetCurrentAnimIndex();
	Super::StartAttack();
	PerformAttack();
}

void UMeleeAttackComp::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	Server_PlayAttackAnim();
	CheckForCollisionWithEnemy();
}

void UMeleeAttackComp::SetCurrentAnimIndex()
{
	if (AttackAnims.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("FirstAttackAnims array is empty!"));
		return;	
	}

	if (!bIsFirstAttackAnimSet || AttackAnims.Num() == 1)
	{
		bIsFirstAttackAnimSet = true;
		return;
	}

	if (AttackAnims.Num() == (CurrentAttackAnimIndex + 1))
	{
		CurrentAttackAnimIndex = 0;
		return;
	}
	++CurrentAttackAnimIndex;
}

void UMeleeAttackComp::Server_PlayAttackAnim_Implementation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	Multicast_PlayAttackAnim();
}

void UMeleeAttackComp::Multicast_PlayAttackAnim_Implementation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	if (AttackAnims[CurrentAttackAnimIndex])
	{
		float PlayRate = 1.f;
		if (AttackAnims[CurrentAttackAnimIndex]->GetPlayLength() > GetAttackCooldown())
		{
			const float AnimLength = AttackAnims[CurrentAttackAnimIndex]->GetPlayLength();
			PlayRate = AnimLength / GetAttackCooldown();
		}
		OwnerCharacter->PlayAnimMontage(AttackAnims[CurrentAttackAnimIndex], PlayRate);
	}
}

void UMeleeAttackComp::PlayAttackAnim()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	if (AttackAnims[CurrentAttackAnimIndex])
	{
		float PlayRate = 1.f;
		if (AttackAnims[CurrentAttackAnimIndex]->GetPlayLength() > GetAttackCooldown())
		{
			const float AnimLength = AttackAnims[CurrentAttackAnimIndex]->GetPlayLength();
			PlayRate = AnimLength / GetAttackCooldown();
		}
		OwnerCharacter->PlayAnimMontage(AttackAnims[CurrentAttackAnimIndex], PlayRate);
	}
}

float UMeleeAttackComp::GetCurrentAnimLength()
{
	if (AttackAnims.Num() == 0)
	{
		return 0.0;
	}

	return AttackAnims[CurrentAttackAnimIndex]->GetPlayLength();
}

void UMeleeAttackComp::CheckForCollisionWithEnemy()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	FVector SweepLocation = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * AttackRadius;

	Sweep(SweepLocation);
}

float UMeleeAttackComp::GetAttackCooldown() const
{
	return AttackCooldown * AttackSpeedModifier;
}

float UMeleeAttackComp::GetDamageAmount() const
{
	return Super::GetDamageAmount() * AttackDamageModifier;
}

void UMeleeAttackComp::Sweep_Implementation(FVector SweepLocation)
{
	if (!OwnerCharacter || SweepLocation == FVector::ZeroVector)
	{
		return;
	}

	TArray<FHitResult> HitResults;

	FCollisionQueryParams QueryParams;

	QueryParams.AddIgnoredActor(OwnerCharacter);
	
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
	
	const bool bHit = GetWorld()->SweepMultiByObjectType(
		HitResults,
		SweepLocation,
		SweepLocation, 
		FQuat::Identity,
		ObjectQueryParams,
		FCollisionShape::MakeSphere(AttackRadius),
		QueryParams);
	
#if WITH_EDITOR
	if (bDrawDebug)
	{
		DrawDebugSphere(
			GetWorld(),
			SweepLocation,
			AttackRadius,
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
				if (APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerCharacter); PlayerCharacter->ImpactParticles)
				{
					SpawnParticles(PlayerCharacter, Hit);
				}
			}
		}
		for (AActor* Actor : UniqueHitActors)
		{
			UGameplayStatics::ApplyDamage(
				Actor,
				GetDamageAmount(),
				OwnerCharacter->GetController(),
				OwnerCharacter,
				UDamageType::StaticClass()
				);
			UE_LOG(LogTemp, Log, TEXT("%s hit for %f damage"), *Actor->GetName(), GetDamageAmount());
		}
	}
}




