#include "SlashAttackComp.h"

#include "EnemyBase.h"
#include "../SpecialAttackComps/ShadowStrikeAttackComp.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"


USlashAttackComp::USlashAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	DamageAmount = 15.f;
	AttackCooldown = 0.25f;
}

void USlashAttackComp::StartAttack()
{
	if (!bCanAttack)
	{
		return;
	}
	/*if (const float AttackAnimLength = AttackMontage ? AttackMontage->GetPlayLength() : 0.f; AttackAnimLength > 0.0f)
	{
		SetAttackCooldown(AttackAnimLength);
	}*/
	
	Super::StartAttack();

	if (!Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	PerformAttack();
}

void USlashAttackComp::StartAttack(const float NewDamageAmount, const float NewAttackCooldown)
{
	if (!bCanAttack)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Calling Super StartAttack(float), DamageAmount is %f"), GetDamageAmount());
	Super::StartAttack(NewDamageAmount, NewAttackCooldown);
	UE_LOG(LogTemp, Warning, TEXT("Called Super StartAttack(float), DamageAmount is %f"), GetDamageAmount());

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
			AttackAnimLength/3.5,
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

float USlashAttackComp::GetAttackCooldown() const
{
	return Super::GetAttackCooldown() * AttackSpeedModifier;
}

float USlashAttackComp::GetDamageAmount() const
{
	return Super::GetDamageAmount() * AttackDamageModifier;
}

void USlashAttackComp::Sweep_Implementation(FVector SweepLocation)
{
	if (!OwnerCharacter || SweepLocation == FVector::ZeroVector)
	{
		return;
	}

	TArray<FHitResult> HitResults;
	
	FHitResult HitResult;

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
		QueryParams
		);
	
	if (bHit)
	{
		TArray<AActor*> HitActors;
		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.GetActor() && !HitActors.Contains(Hit.GetActor()))
			{
				HitActors.Add(Hit.GetActor());
				if (!Hit.GetActor()->IsA(APlayerCharacterBase::StaticClass()))
				{
					SpawnParticles(Cast<APlayerCharacterBase>(GetOwner()), Hit);
				}
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
				GetDamageAmount(),
				OwnerCharacter->GetController(),
				OwnerCharacter,
				UDamageType::StaticClass()
				);
			/*DrawDebugSphere(GetWorld(), Actor->GetActorLocation(),
		AttackRadius, 32, FColor::Purple, false, 5.f);
			UE_LOG(LogTemp, Warning, TEXT("%s hit %s for %f damage"), *OwnerCharacter->GetName(), *Actor->GetName(), GetDamageAmount());*/
		}
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
		OwnerCharacter->PlayAnimMontage(AttackMontage, 2);
	}
}
