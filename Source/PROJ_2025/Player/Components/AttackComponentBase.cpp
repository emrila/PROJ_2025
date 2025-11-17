#include "AttackComponentBase.h"

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


void UAttackComponentBase::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UAttackComponentBase::ResetAttackCooldown()
{
	bCanAttack = true;
}

void UAttackComponentBase::SpawnParticles_Implementation(APlayerCharacterBase* PlayerCharacter, FHitResult Hit)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PlayerCharacter->ImpactParticles, Hit.ImpactPoint);
	
}
