// Furkan approves of this

#include "BTT_MeleeMove.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"

#include "GameFramework/CharacterMovementComponent.h"


#include "MushroomCharacter.h"

UBTT_MeleeMove::UBTT_MeleeMove()
{
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_MeleeMove::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	OwnerComponent = &OwnerComp;
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;

	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn)
		return EBTNodeResult::Failed;

	Mushroom = AIPawn;

	Blackboard = OwnerComp.GetBlackboardComponent();
	if (!Blackboard)
		return EBTNodeResult::Failed;

	TargetActor = Cast<AActor>(Blackboard->GetValueAsObject("TargetActor"));
	float Dist = FVector::Dist(Mushroom->GetActorLocation(), TargetActor->GetActorLocation());
	if (Dist <= 55.f)
	{
			Super::FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return EBTNodeResult::Succeeded;
	}
	Super::ExecuteTask(OwnerComp, NodeMemory);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandleTimeout);
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UBTT_MeleeMove::CheckIfOutOfRangeFromTarget, 0.05f, true);
	
	GetWorld()->GetTimerManager().SetTimer(TimerHandleTimeout, this, &UBTT_MeleeMove::TimedOut, TimeOutTimer, false);

	return EBTNodeResult::InProgress;
}

void UBTT_MeleeMove::CheckIfOutOfRangeFromTarget()
{
	if (!Mushroom || !TargetActor)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandleTimeout);
		Blackboard->SetValueAsBool("MeleeAttack", false);
		if (AMushroomCharacter* MushroomChar = Cast<AMushroomCharacter>(Mushroom))
		{
			MushroomChar->MeleeRun = false;
			MushroomChar->GetCharacterMovement()->MaxWalkSpeed = 600.f;
		}
		FinishLatentTask(*OwnerComponent, EBTNodeResult::Failed);
		return;
	}
	float Distance = FVector::Dist(Mushroom->GetActorLocation(), TargetActor->GetActorLocation());
	UE_LOG(LogTemp, Warning, TEXT("%f"), Distance);
	if (Distance >= InterruptRange)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(TimerHandleTimeout);
		Blackboard->SetValueAsBool("MeleeAttack", false);
		if (AMushroomCharacter* MushroomChar = Cast<AMushroomCharacter>(Mushroom))
		{
			MushroomChar->MeleeRun = false;
			MushroomChar->GetCharacterMovement()->MaxWalkSpeed = 600.f;
		}
		FinishLatentTask(*OwnerComponent, EBTNodeResult::Failed);
	} 
}

void UBTT_MeleeMove::TimedOut()
{

	UE_LOG(LogTemp, Error, TEXT("TimedOut"));
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	Blackboard->SetValueAsBool("MeleeAttack", false);
	if (AMushroomCharacter* MushroomChar = Cast<AMushroomCharacter>(Mushroom))
	{
		MushroomChar->MeleeRun = false;
		MushroomChar->GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
	GetWorld()->GetTimerManager().ClearTimer(TimerHandleTimeout);

	FinishLatentTask(*OwnerComponent, EBTNodeResult::Failed);

}

