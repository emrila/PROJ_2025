// Furkan approves of this


#include "CactusCharacter.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void ACactusCharacter::Server_ShootProjectile_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation);
}

void ACactusCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AAIController* AICon = Cast<AAIController>(GetController());
	if (!AICon) return;

	UBlackboardComponent* BBComp = AICon->GetBlackboardComponent();
	if (!BBComp) return;
	
	AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject("TargetActor"));
	if (TargetActor)
	{
		FVector Direction = TargetActor->GetActorLocation() - GetActorLocation();
		Direction.Z = 0;

		if (!Direction.IsNearlyZero())
		{
			FRotator TargetRot = Direction.Rotation();
			FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaSeconds, 1.5f);
			SetActorRotation(NewRot);
		}
	}
}
