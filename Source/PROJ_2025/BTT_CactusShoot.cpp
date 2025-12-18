// Furkan approves of this


#include "BTT_CactusShoot.h"

#include "AIController.h"
#include "CactusCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTT_CactusShoot::UBTT_CactusShoot()
{
	NodeName = "Shoot";
}

EBTNodeResult::Type UBTT_CactusShoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;
	
	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
		return EBTNodeResult::Failed;
	
	UObject* TargetObject = BlackboardComp->GetValueAsObject(GetSelectedBlackboardKey());
	AActor* TargetActor = Cast<AActor>(TargetObject);

	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	const FVector AI_Location = AIPawn->GetActorLocation();
	const FVector Target_Location = TargetActor->GetActorLocation();

	FRotator RotToPlayer = UKismetMathLibrary::FindLookAtRotation(AI_Location, Target_Location);
	ACactusCharacter* CactusCharacter = Cast<ACactusCharacter>(AIPawn);
	
	FRotator ShootRot = FRotator(RotToPlayer.Pitch, AIPawn->GetActorRotation().Yaw, AIPawn->GetActorRotation().Roll);

	CactusCharacter->Server_ShootProjectile(Target_Location);
	
	return EBTNodeResult::Succeeded;
}
