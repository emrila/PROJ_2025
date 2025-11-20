// Furkan approves of this


#include "Golem.h"

#include "Net/UnrealNetwork.h"

AGolem::AGolem()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AGolem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGolem, RageBuildup);
}

float AGolem::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator,
	AActor* DamageCauser)
{
	if (Health > 0)
	{
		return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	return 0;
}

void AGolem::HandleHit()
{
	Super::HandleHit();
	RageBuildup += 10;
}

void AGolem::HandleDeath()
{
	if (CombatManager && CombatManager->HasAuthority())
	{
		CombatManager->RegisterEnemyDeath();
		UE_LOG(LogTemp, Log, TEXT("EnemyBase: Enemy died"));
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[this] ()
			{
				Die();
			},
			(5),
			false
			);		
	OnRep_Death();
}

void AGolem::Die()
{
	Destroy();
}


