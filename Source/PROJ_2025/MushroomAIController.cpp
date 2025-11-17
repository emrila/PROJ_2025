// Furkan approves of this


#include "MushroomAIController.h"

#include "MushroomCharacter.h"
#include "Net/UnrealNetwork.h"




void AMushroomAIController::StartAttackCooldown()
{
	if (Blackboard)
	{
		Cast<AMushroomCharacter>(GetPawn())->AttackIsOnCooldown = true;
		Blackboard->SetValueAsBool("AttackIsOnCooldown", true);
	}

	GetWorldTimerManager().SetTimer(
		AttackCooldownHandle,
		this,
		&AMushroomAIController::ResetAttackCooldown,
		2.5f,
		false
	);
}

void AMushroomAIController::ResetAttackCooldown()
{
	if (Blackboard)
	{
		Cast<AMushroomCharacter>(GetPawn())->AttackIsOnCooldown = false;
		Blackboard->SetValueAsBool("AttackIsOnCooldown", false);
	}
}