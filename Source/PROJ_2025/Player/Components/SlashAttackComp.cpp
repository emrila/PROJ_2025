#include "SlashAttackComp.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"


USlashAttackComp::USlashAttackComp()
{

	PrimaryComponentTick.bCanEverTick = true;
}

void USlashAttackComp::StartAttack()
{
	if (!bCanAttack)
	{
		return;
	}
	if (const float AttackAnimLength = AttackMontage ? AttackMontage->GetPlayLength() : 0.f; AttackAnimLength > 0.0f)
	{
		SetAttackCoolDown(AttackAnimLength);
	}
	
	Super::StartAttack();

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	PerformAttack();
}


void USlashAttackComp::BeginPlay()
{
	Super::BeginPlay();
	
}

void USlashAttackComp::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	if (const float AttackAnimLength = AttackMontage ? AttackMontage->GetPlayLength() : 0.f; AttackAnimLength > 0.0f)
	{
		if (GetWorld()->GetTimerManager().IsTimerActive(SweepTimerHandle))
		{
			return;
		}
		
		Server_PlayAttackAnim();

		GetWorld()->GetTimerManager().SetTimer(
			SweepTimerHandle,
			this,
			&USlashAttackComp::CheckForCollisionWithEnemies,
			AttackAnimLength * 0.5f,
			false
			);
	}
}

void USlashAttackComp::CheckForCollisionWithEnemies()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	if (const APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(OwnerCharacter))
	{
		const AActor* RightHandActor = PlayerCharacter->GetRightHandAttachedActor();
		if (!RightHandActor)
		{
			UE_LOG(LogTemp, Error, TEXT("%s, RightHandActor is NULL!"), *FString(__FUNCTION__));
			return;
		}
		const FVector SweepLocation = RightHandActor->GetActorLocation();
		Sweep(SweepLocation);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("%s, Unable to cast OwnerCharacter to APlayerCharacterBase"), *FString(__FUNCTION__));
}

void USlashAttackComp::Sweep(FVector SweepLocation)
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
		SweepLocation,  //+ FVector(0.f, 0.f, 1.f), // Slight offset to avoid zero-length sweep
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRadius),
		QueryParams
		);

	if (bHit)
	{
		TArray<AActor*> HitActors;
		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.GetActor())  //&& Hit.GetActor() != OwnerCharacter
			{
				HitActors.Add(Hit.GetActor());
			}
		}

		for (AActor* Actor : HitActors)
		{
			if (Actor->IsA(APlayerCharacterBase::StaticClass()))
			{
				continue;
			}
			UGameplayStatics::ApplyDamage(
				Actor,
				DamageAmount,
				OwnerCharacter->GetController(),
				OwnerCharacter,
				UDamageType::StaticClass()
				);

			UE_LOG(LogTemp, Warning, TEXT("%s hit %s for %f damage"), *OwnerCharacter->GetName(), *Actor->GetName(), DamageAmount);

			DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), AttackRadius, 12, FColor::Red, false, 5.0f);
		}
	}
	else
	{
		DrawDebugSphere(GetWorld(), SweepLocation, AttackRadius, 12, FColor::Green, false, 5.0f);
	}
}

void USlashAttackComp::Server_PlayAttackAnim_Implementation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	Multicast_PlayAttackAnim();
}

void USlashAttackComp::Multicast_PlayAttackAnim_Implementation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	if (AttackMontage)
	{
		OwnerCharacter->PlayAnimMontage(AttackMontage);
	}
}



