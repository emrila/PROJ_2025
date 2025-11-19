#include "AttackComponentBase.h"

#include "EnhancedInputComponent.h"
#include "MeleeAttackComp.h"
#include "GameFramework/Character.h"


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

void UAttackComponentBase::SpawnParticles_Implementation(APlayerCharacterBase* PlayerCharacter, FHitResult Hit)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PlayerCharacter->ImpactParticles, Hit.ImpactPoint);
	
}
