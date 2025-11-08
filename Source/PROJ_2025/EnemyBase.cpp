// Furkan approves of this


#include "EnemyBase.h"

#include "Net/UnrealNetwork.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

}


float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	LaunchCharacter(GetActorForwardVector() * -1555, false, false);
	if (Health <= 0)
	{
		HandleDeath();
	}

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void AEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEnemyBase, Health);
}

void AEnemyBase::HandleDeath()
{
	if (CombatManager && CombatManager->HasAuthority())
	{
		CombatManager->RegisterEnemyDeath();
		UE_LOG(LogTemp, Log, TEXT("EnemyBase: Enemy died"));
	}

	Destroy();
}





