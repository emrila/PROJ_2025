// Furkan approves of this


#include "BTT_FindOrChangeTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Player/Characters/PlayerCharacterBase.h"

UBTT_FindOrChangeTarget::UBTT_FindOrChangeTarget()
{
	NodeName = "Find Or Change Target";
}

EBTNodeResult::Type UBTT_FindOrChangeTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
	{
		return EBTNodeResult::Failed;
	}
	AActor* CurrentTarget = Cast<AActor>(Blackboard->GetValueAsObject(GetSelectedBlackboardKey()));
	
	TArray<AActor*> PlayerPawns;
	for (FConstPlayerControllerIterator It = AIPawn->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn() && PC->GetPawn() != AIPawn)
		{
			PlayerPawns.Add(PC->GetPawn());
		}
	}
	
	TArray<AActor*> VisiblePlayers;
	for (AActor* Player : PlayerPawns)
	{
		if (Player && AICon->LineOfSightTo(Player))
		{
			VisiblePlayers.Add(Player);
		}
	}
	
	// DEN HAR REDAN TARGET ---------------------------
	if (CurrentTarget) 
	{
		if (VisiblePlayers.Num() == 0) // Hitta inga targets så den har kvar den den hade
		{
			return EBTNodeResult::Succeeded;
		}
		if (VisiblePlayers.Num() > 0 && !VisiblePlayers.Contains(CurrentTarget)) // Hittade ny(a) target(s) och den har inte LOS till den gamla längre, då väljer den en ny direkt
		{
			AActor* NewTarget = VisiblePlayers[FMath::RandRange(0, VisiblePlayers.Num() - 1)];
			if (APlayerCharacterBase* PCB = Cast<APlayerCharacterBase>(NewTarget))
			{
				if (PCB->IsAlive())
				{
					Blackboard->SetValueAsObject(GetSelectedBlackboardKey(), NewTarget);
					return EBTNodeResult::Succeeded;
				}
			}
			return EBTNodeResult::Succeeded;
		}
		bool bTryToSwitch = FMath::FRand() <= SwitchChance;
		if (bTryToSwitch && VisiblePlayers.Num() > 1 && VisiblePlayers.Contains(CurrentTarget)) // Hittade ny(a) och den har kvar den gamla, då är det 10% att den byter här
		{
			VisiblePlayers.Remove(CurrentTarget);
			AActor* NewTarget = VisiblePlayers[FMath::RandRange(0, VisiblePlayers.Num() - 1)];
			if (APlayerCharacterBase* PCB = Cast<APlayerCharacterBase>(NewTarget))
			{
				if (PCB->IsAlive())
				{
					Blackboard->SetValueAsObject(GetSelectedBlackboardKey(), NewTarget);
					return EBTNodeResult::Succeeded;
				}
			}
			return EBTNodeResult::Succeeded;
		}
		return EBTNodeResult::Succeeded; 
	}
	
	// OM DEN INTE HAR EN TARGET ---------------------------
	if (VisiblePlayers.Num() > 0) // Finns line of sight till nån så den tar den :)
		{
			AActor* NewTarget = VisiblePlayers[FMath::RandRange(0, VisiblePlayers.Num() - 1)];

			if (APlayerCharacterBase* PCB = Cast<APlayerCharacterBase>(NewTarget))
			{
				if (PCB->IsAlive())
				{
					Blackboard->SetValueAsObject(GetSelectedBlackboardKey(), NewTarget);
					return EBTNodeResult::Succeeded;
				}
			}
			return EBTNodeResult::Failed;
		}
	if (PlayerPawns.Num() > 0)
	{
		AActor* NewTarget = PlayerPawns[FMath::RandRange(0, PlayerPawns.Num() - 1)];

		if (APlayerCharacterBase* PCB = Cast<APlayerCharacterBase>(NewTarget))
		{
			if (PCB->IsAlive())
			{
				Blackboard->SetValueAsObject(GetSelectedBlackboardKey(), NewTarget);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	
	if (APlayerCharacterBase* PCB = Cast<APlayerCharacterBase>(CurrentTarget))
	{
		if (!PCB->IsAlive())
		{
			Blackboard->SetValueAsObject(GetSelectedBlackboardKey(), nullptr);
		}
	}
	return EBTNodeResult::Failed; // Här är det lite rip. Den får försöka igen ig
}
