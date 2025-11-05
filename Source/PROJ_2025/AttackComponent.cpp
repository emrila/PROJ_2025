

#include "AttackComponent.h"

#include "MageProjectile.h"
#include "GameFramework/Character.h"


UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttackComponent::StartAttack()
{
	if (!bCanAttack || !OwnerCharacter || !ProjectileClass)
	{
		return;
	}

	if (!OwnerCharacter || !ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackComponent, OwnerCharacter or ProjectileClass is NULL!"));
		return;
	}
	


	GetWorld()->GetTimerManager().SetTimer(
		AttackCoolDownTimerHandle,
		this,
		&UAttackComponent::ResetAttackCooldown,
		AttackCoolDown,
		false
		);
}


void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	//check(OwnerCharacter != nullptr);
	
}

void UAttackComponent::ResetAttackCooldown()
{
	bCanAttack = true;
}

