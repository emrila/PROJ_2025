// Furkan approves of this


#include "BTT_LookAt.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTT_LookAt::UBTT_LookAt()
{
	NodeName = "Look At Target";
}

EBTNodeResult::Type UBTT_LookAt::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}
	
	UObject* TargetObject = BlackboardComp->GetValueAsObject(GetSelectedBlackboardKey());
	AActor* TargetActor = Cast<AActor>(TargetObject);

	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	const FVector AI_Location = AIPawn->GetActorLocation();
	const FVector Target_Location = TargetActor->GetActorLocation();

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(AI_Location, Target_Location);
	LookAtRotation.Pitch = 0.f; 
	
	AIPawn->SetActorRotation(LookAtRotation);

	return EBTNodeResult::Succeeded;
}

