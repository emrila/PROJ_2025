// Furkan approves of this


#include "BTT_BomberAttack.h"

#include "BomberAIController.h"
#include "BomberCharacter.h"
#include "GameFramework/Character.h"    
#include "BehaviorTree/BlackboardComponent.h"

#include "Kismet/KismetMathLibrary.h"

UBTT_BomberAttack::UBTT_BomberAttack()
{
	NodeName = "Bomber Attack";
}

EBTNodeResult::Type UBTT_BomberAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon)
		return EBTNodeResult::Failed;
	
	APawn* AIPawn = AICon->GetPawn();
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	ABomberCharacter* BomberCharacter = Cast<ABomberCharacter>(AIPawn);
	
	ABomberAIController* BomberAICon = Cast<ABomberAIController>(AICon);
	if (!BomberAICon)
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
		
		FVector LaunchVelocity = JumpDir * BomberAICon->MoveSpeed*2.8 + FVector(0, 0, BomberAICon->JumpHeight*1.5);

		Character->LaunchCharacter(LaunchVelocity, true, true);
	}
	BomberCharacter->bIsDiving = true;
	return EBTNodeResult::Succeeded;
}
