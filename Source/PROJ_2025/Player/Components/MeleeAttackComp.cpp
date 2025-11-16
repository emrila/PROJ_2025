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
		SetAttackCoolDown(Delay);
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
			(Delay - 0.3f),
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
		OwnerCharacter->PlayAnimMontage(AttackAnims[CurrentAttackAnimIndex]);
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
		OwnerCharacter->PlayAnimMontage(AttackAnims[CurrentAttackAnimIndex]);
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
	FVector SweepLocation2 = FVector::ZeroVector;

	if (const APlayerCharacterBase* PlayerCharacterBase = Cast<APlayerCharacterBase>(OwnerCharacter))
	{
		if (PlayerCharacterBase->GetRightHandSocketLocation() != FVector::ZeroVector)
		{
			SweepLocation1 = PlayerCharacterBase->GetRightHandSocketLocation();
		}
		SweepLocation2 = PlayerCharacterBase->GetLeftHandSocketLocation();
	}

	Sweep(SweepLocation1);
	Sweep(SweepLocation2);
}

void UMeleeAttackComp::Sweep(FVector SweepLocation)
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
	
	if (bHit)
	{
		TArray<AActor*> HitActors;
		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.GetActor())  //&& Hit.GetActor() != OwnerCharacter
			{
				if (Hit.GetActor()->IsA(AEnemyBase::StaticClass()))
				{
					HitActors.Add(Hit.GetActor());
					if (const APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(Hit.GetActor()); PlayerCharacter->ImpactParticles)
					{
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Cast<APlayerCharacterBase>(OwnerCharacter)->ImpactParticles, Hit.ImpactPoint);
					}
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
				DamageAmount,
				OwnerCharacter->GetController(),
				OwnerCharacter,
				UDamageType::StaticClass()
				);
			DrawDebugSphere(GetWorld(), Actor->GetActorLocation(), AttackRadius, 12, FColor::Red, false, 5.0f);
		}
	}
	else
	{
		DrawDebugSphere(GetWorld(), SweepLocation, AttackRadius, 12, FColor::Green, false, 5.0f);
	}
	
}




