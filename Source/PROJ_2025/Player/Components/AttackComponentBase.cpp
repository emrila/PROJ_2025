#include "AttackComponentBase.h"

#include "Core/UpgradeSubsystem.h"
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

	if (OwnerCharacter->IsLocallyControlled())
	{
		if (UUpgradeSubsystem* UpgradeSubsystem = UUpgradeSubsystem::Get(GetWorld()))
		{
			auto Bind = [&UpgradeSubsystem](UObject* InOwner, const FName& InPropertyName)
			{
				UpgradeSubsystem->BindAttribute(InOwner, InPropertyName, InPropertyName, InPropertyName);
				UE_LOG(LogTemp, Log, TEXT("%hs, Bound Attribute: %s"), __FUNCTION__, *InPropertyName.ToString());
			};
			Bind(this,  "DamageAmount");
			Bind(this,  "AttackCoolDown");
		}
	}
}

void UAttackComponentBase::ResetAttackCooldown()
{
	bCanAttack = true;
}
