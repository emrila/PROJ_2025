#include "AttackComponentBase.h"

#include "EnhancedInputComponent.h"
#include "MeleeAttackComp.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"


UAttackComponentBase::UAttackComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttackComponentBase::StartAttack()
{
	if (!bCanAttack)
	{
		return;
	}

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComponentBase, OwnerCharacter is NULL!"));
		return;
	}

	bCanAttack = false;

	GetWorld()->GetTimerManager().SetTimer(
		AttackCoolDownTimerHandle,
		this,
		&UAttackComponentBase::ResetAttackCooldown,
		AttackCoolDown,
		false
		);
}

void UAttackComponentBase::StartAttack(const float NewDamageAmount)
{
	if (!bCanAttack)
	{
		return;
	}

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComponentBase, OwnerCharacter is NULL!"));
		return;
	}
	
	DamageAmountToStore = DamageAmount;
	DamageAmount = NewDamageAmount;

	bCanAttack = false;

	GetWorld()->GetTimerManager().SetTimer(
		AttackCoolDownTimerHandle,
		this,
		&UAttackComponentBase::ResetAttackCooldown,
		AttackCoolDown,
		false
		);
}

void UAttackComponentBase::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp,
                                                  UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &UAttackComponentBase::StartAttack);
	}
}

void UAttackComponentBase::Server_SpawnEffect_Implementation(const FVector& EffectSpawnLocation, UNiagaraSystem* Effect)
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	
	if (!Effect)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, Effect is NULL!"), *FString(__FUNCTION__));
		return;
	}
	if (!OwnerCharacter->HasAuthority() || EffectSpawnLocation.IsNearlyZero())
	{
		return;
	}
	Multicast_SpawnEffect_Implementation(EffectSpawnLocation, Effect);
}

void UAttackComponentBase::Multicast_SpawnEffect_Implementation(const FVector& EffectSpawnLocation, UNiagaraSystem* Effect)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Effect, EffectSpawnLocation);
}

void UAttackComponentBase::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UAttackComponentBase::ResetAttackCooldown()
{
	bCanAttack = true;
	
	if (DamageAmountToStore > 0.f)
	{
		DamageAmount = DamageAmountToStore;
		DamageAmountToStore = 0.f;
	}
}

void UAttackComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, OwnerCharacter);
}

void UAttackComponentBase::SpawnParticles_Implementation(APlayerCharacterBase* PlayerCharacter, FHitResult Hit)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PlayerCharacter->ImpactParticles, Hit.ImpactPoint);
	
}
