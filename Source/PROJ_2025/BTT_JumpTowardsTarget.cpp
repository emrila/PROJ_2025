// Furkan approves of this


#include "BTT_JumpTowardsTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"    
#include "MushroomAIController.h"
#include "MushroomCharacter.h"

UBTT_JumpTowardsTarget::UBTT_JumpTowardsTarget()
{
	NodeName = "Jump Towards Target";
}


EBTNodeResult::Type UBTT_JumpTowardsTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	int CurrentIndex = 9;
	float LastWorkingAngle = 180.f;
	bool Increment = FMath::RandBool();
	TArray<int32> CheckedAngle;

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
	while (CheckedAngle.Num() < 17)
	{
		FJumpDirection Direction = MushroomAICon->JumpDirections[CurrentIndex];
		if (!TestDirection(Direction, RotToPlayer, AIPawn))
		{
			float Rand = FMath::FRand();
			UE_LOG(LogTemp, Display, TEXT("Rand: %f"), Rand);
			UE_LOG(LogTemp, Display, TEXT("ChanceToJump: %f"), Direction.ChanceToStay);
			bool isGonnaJump = FMath::FRand() <= Direction.ChanceToStay;
			
			if (isGonnaJump)
			{
				//Jump to current
				Jump(Direction.Angle, RotToPlayer, AIPawn, MushroomAICon->JumpHeight, MushroomAICon->MoveSpeed);
				UE_LOG(LogTemp, Warning, TEXT("Jumping"));
				return EBTNodeResult::Succeeded;
			}
			LastWorkingAngle = Direction.Angle;
			if (Increment)
			{
				CurrentIndex++;
			}else
			{
				CurrentIndex--;
			}
		}else
		{
			if (LastWorkingAngle != 180.f)
			{
				Jump(LastWorkingAngle, RotToPlayer, AIPawn, MushroomAICon->JumpHeight, MushroomAICon->MoveSpeed);
				UE_LOG(LogTemp, Warning, TEXT("Jumping"));
				return EBTNodeResult::Succeeded;
			}
			CheckedAngle.Add(LastWorkingAngle);
			if (Increment)
			{
				CurrentIndex++;
				if (CurrentIndex == 17)
				{
					CurrentIndex = 8;
					Increment = !Increment;
				}
			}else
			{
				CurrentIndex--;
				if (CurrentIndex == -1)
				{
					CurrentIndex = 10;
					Increment = !Increment;
				}
			}
		}
	}
	
	
	return EBTNodeResult::Succeeded;

	
}

bool UBTT_JumpTowardsTarget::TestDirection(const FJumpDirection Direction, FRotator RotationToPlayer, const APawn* Pawn)
{
	RotationToPlayer.Yaw += Direction.Angle; 

	FVector NormalizedDir = RotationToPlayer.Vector().GetSafeNormal();
	
	FVector Start = Pawn->GetActorLocation() + FVector(0, 0, 100);
	FVector End = Start + NormalizedDir * 300.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Pawn);

	bool bHit = Pawn->GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);
	
	//DrawDebugLine(Pawn->GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 1.f, 0, 2.f);
	
	return bHit;
}


void UBTT_JumpTowardsTarget::Jump(float Angle, FRotator RotationToPlayer, APawn* Pawn, float JumpStrength, float ForwardStrength)
{
	if (Pawn->HasAuthority())
	{
		AMushroomCharacter* MushroomChar = Cast<AMushroomCharacter>(Pawn);
		if (MushroomChar)
		{
			MushroomChar->Multicast_Jump(Angle, RotationToPlayer, JumpStrength, ForwardStrength);
		}
	}
}

