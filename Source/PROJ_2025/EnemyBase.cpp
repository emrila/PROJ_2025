// Furkan approves of this


#include "EnemyBase.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "WizardPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Controllers/PlayerControllerBase.h"


AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

}


float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
		return 0.f;
	
	Health -= DamageAmount;
	APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(DamageCauser);
	if (Player)
	{
		AWizardPlayerState* WizardPlayerState = Cast<AWizardPlayerState>(Player->GetPlayerState());
		if (!WizardPlayerState)
		{
			return 0.f;
		}
		
		if (!HasDied)
		{
			WizardPlayerState->AddDamageDealt(DamageAmount);
		}
	}
	HandleHit(DamageEvent, DamageCauser);
	OnRep_Health();
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
	DOREPLIFETIME(AEnemyBase, HasDied);
	DOREPLIFETIME(AEnemyBase, UpdateHealthbar)
}

void AEnemyBase::OnRep_Health() const
{
	UpdateHealthbar.Broadcast(Health);
}

void AEnemyBase::FinishDeath()
{
	SpawnDeathEffect();
	Death();
	Destroy();
}

void AEnemyBase::HandleDeath()
{
	if (!HasAuthority())
		return;
	
	if (CombatManager && !HasDied)
	{
		CombatManager->RegisterEnemyDeath();
		UE_LOG(LogTemp, Log, TEXT("EnemyBase: Enemy died"));
		HasDied = true;
	}else
	{
		UE_LOG(LogTemp, Error, TEXT("COMBATMANAGER IS NULL"));
	}
	if (DeathMontage )
	{	
		if (AController* ControllerNullCheck = GetController())
		{
			AAIController* AICon = Cast<AAIController>(ControllerNullCheck);
			if (AICon)
			{
				if (AICon->BrainComponent)
				{
					AICon->BrainComponent->StopLogic("Enemy died");
				}
			}
			UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
			if (AnimInst)
			{
				float PlayRate = 3.f;
				float Duration = AnimInst->Montage_Play(DeathMontage, PlayRate);
				if (Duration <= 0.f)
				{
					SpawnDeathEffect();
					Destroy();
					return;
				}
				FTimerHandle DeathTimerHandle;

				GetWorld()->GetTimerManager().SetTimer(
					DeathTimerHandle,
					this,
					&AEnemyBase::FinishDeath,
					(Duration/PlayRate) - 0.4f,
					false
				);
				return;
			}else
			{
				SpawnDeathEffect();
				Death();
				Destroy();
			}
		}
		return;
	}
	SpawnDeathEffect();
	Death();
	Destroy();
}

void AEnemyBase::SpawnDeathEffect_Implementation()
{
	if (DeathEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DeathEffect, GetActorLocation());
	}
}

void AEnemyBase::HandleHit_Implementation(struct FDamageEvent const& DamageEvent, AActor* DamageCauser)
{
	HitFeedback();
}







