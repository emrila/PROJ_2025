// Furkan approves of this


#include "BTT_CactusSpikeExplosion.h"

#include "AIController.h"
#include "CactusCharacter.h"

UBTT_CactusSpikeExplosion::UBTT_CactusSpikeExplosion()
{
	NodeName = "Cactus Spike Explosion";
}

EBTNodeResult::Type UBTT_CactusSpikeExplosion::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
	{
		return EBTNodeResult::Failed;
	}

	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	ACactusCharacter* Cactus = Cast<ACactusCharacter>(AIPawn);
	if (!Cactus)
	{
		return EBTNodeResult::Failed;
	}
	Cactus->Server_SpawnSpikeExplosion(Cactus->GetActorLocation(), Cactus->GetActorRotation());
	
	return EBTNodeResult::Succeeded;
}
