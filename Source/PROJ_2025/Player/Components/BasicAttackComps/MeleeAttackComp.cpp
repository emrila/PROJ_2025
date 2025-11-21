#include "MeleeAttackComp.h"

#include "EnemyBase.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"

UMeleeAttackComp::UMeleeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UMeleeAttackComp::BeginPlay()
{
	Super::BeginPlay();

	bIsFirstAttackAnimSet = false;
}

void UMeleeAttackComp::StartAttack()
{
	if (!bCanAttack)
	{
		return;
	}

	SetCurrentAnimIndex();

	if (const float Delay = GetCurrentAnimLength(); Delay > 0.0f)
	{
		SetAttackCooldown(GetAttackCooldown());
	}
	
	Super::StartAttack();

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	PerformAttack();
}

void UMeleeAttackComp::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	if (const float Delay = GetCurrentAnimLength(); Delay > 0.0f)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(SweepTimerHandle))
		{
			return;
		}
		
		Server_PlayAttackAnim();

		GetWorld()->GetTimerManager().SetTimer(
			SweepTimerHandle,
			[this] ()
			{
				CheckForCollisionWithEnemy();
			},
			(Delay/2.f - 0.3f),
			false
			);		
	}
	
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
		OwnerCharacter->PlayAnimMontage(AttackAnims[CurrentAttackAnimIndex], 2.0f);
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
		OwnerCharacter->PlayAnimMontage(AttackAnims[CurrentAttackAnimIndex], 2.0f);
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

	FVector SweepLocation1 = OwnerCharacter->GetActorLocation();
	//FVector SweepLocation2 = FVector::ZeroVector;

	if (const APlayerCharacterBase* PlayerCharacterBase = Cast<APlayerCharacterBase>(OwnerCharacter))
	{
		if (PlayerCharacterBase->GetRightHandSocketLocation() != FVector::ZeroVector)
		{
			SweepLocation1 = PlayerCharacterBase->GetRightHandSocketLocation();
		}
		//SweepLocation2 = PlayerCharacterBase->GetLeftHandSocketLocation();
	}

	Sweep(SweepLocation1);
	//TODO : Determine which sweep locations to use based on the attack animation (one-handed, two-handed, etc.)
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

	const bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		SweepLocation,
		SweepLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRadius),
		QueryParams
		);
	
	TSet<AActor*> UniqueHitActors;
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.GetActor() && Hit.GetActor()->IsA(AEnemyBase::StaticClass()))
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
				DamageAmount,
				OwnerCharacter->GetController(),
				OwnerCharacter,
				UDamageType::StaticClass()
				);
			UE_LOG(LogTemp, Log, TEXT("%s hit for %f damage"), *Actor->GetName(), DamageAmount);
			DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), AttackRadius, 12, FColor::Red, false, 5.0f);
		}
	}
	
	if (UniqueHitActors.Num() == 0)
	{
		DrawDebugSphere(GetWorld(), SweepLocation, AttackRadius, 12, FColor::Green, false, 5.0f);
	}
}




