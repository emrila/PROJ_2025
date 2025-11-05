// Furkan approves of this


#include "MushroomAIController.h"



void AMushroomAIController::StartAttackCooldown()
{
	if (Blackboard)
	{
		Blackboard->SetValueAsBool("AttackIsOnCooldown", true);
		UE_LOG(LogTemp, Warning, TEXT("ATTACK COOLDOWN"));
	}

	GetWorldTimerManager().SetTimer(
		AttackCooldownHandle,
		this,
		&AMushroomAIController::ResetAttackCooldown,
		2.0f,
		false
	);
}

void AMushroomAIController::ResetAttackCooldown()
{
	if (Blackboard)
	{
		Blackboard->SetValueAsBool("AttackIsOnCooldown", false);
	}
}