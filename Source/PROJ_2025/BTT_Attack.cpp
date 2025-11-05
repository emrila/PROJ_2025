// Furkan approves of this


#include "BTT_Attack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"    
#include "MushroomAIController.h"
#include "Kismet/KismetMathLibrary.h"

UBTT_Attack::UBTT_Attack()
{
	NodeName = "Attack Target";
}

EBTNodeResult::Type UBTT_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;
	
	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}
	AMushroomAIController* MushroomAICon = Cast<AMushroomAIController>(AICon);
	if (!MushroomAICon)
		return EBTNodeResult::Failed;
	
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
	RotToPlayer.Pitch = 0.f;

	FVector JumpDir = RotToPlayer.Vector().GetSafeNormal();

	AIPawn->SetActorRotation(RotToPlayer);

	ACharacter* Character = Cast<ACharacter>(AIPawn);
	if (Character && Character->GetCharacterMovement())
	{
		
		FVector LaunchVelocity = JumpDir * MushroomAICon->MoveSpeed*1.8 + FVector(0, 0, MushroomAICon->JumpHeight*1.2);

		Character->LaunchCharacter(LaunchVelocity, true, true);
	}
	MushroomAICon->StartAttackCooldown();
	return EBTNodeResult::Succeeded;
}
