#include "SlashAttackComp.h"
#include "EnhancedInputComponent.h"
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

void USlashAttackComp::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (bIsAttacking)
	{
		StartAttack();
	}
}

void USlashAttackComp::StartAttack()
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
	
	Super::StartAttack();
	PerformAttack();
}

void USlashAttackComp::StartAttack(const float NewDamageAmount, const float NewAttackCooldown)
{
	if (!bCanAttack)
	{
		return;
	}
	
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	if (!OwnerCharacter->HasAuthority())
	{
		Server_StartAttack(NewDamageAmount, NewAttackCooldown);
		return;
	}
	
	Super::StartAttack(NewDamageAmount, NewAttackCooldown);
	
	PerformAttack();
}

void USlashAttackComp::Server_StartAttack_Implementation(const float NewDamageAmount, float NewAttackCooldown)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	Super::StartAttack(NewDamageAmount, NewAttackCooldown);
	
	PerformAttack();
}

void USlashAttackComp::BeginPlay()
{
	Super::BeginPlay();
	
}

void USlashAttackComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &USlashAttackComp::OnStartAttack);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this, &USlashAttackComp::OnEndAttack);
	}
}

void USlashAttackComp::OnStartAttack(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}
	
	bIsAttacking = true;
}

void USlashAttackComp::OnEndAttack(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}
	if (!bIsAttacking)
	{
		return;
	}
	bIsAttacking = false;
}

void USlashAttackComp::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	
	CheckForCollisionWithEnemies();
	Server_PlayAttackAnim();
}

void USlashAttackComp::CheckForCollisionWithEnemies()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}

	Sweep(OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * AttackRadius);
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
		float PlayRate = 1.f;
		if (AttackMontage->GetPlayLength() > GetAttackCooldown())
		{
			const float AnimLength = AttackMontage->GetPlayLength();
			PlayRate = AnimLength / GetAttackCooldown();
		}
		OwnerCharacter->PlayAnimMontage(AttackMontage, PlayRate);
	}
}
