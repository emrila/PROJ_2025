// Furkan approves of this


#include "BomberCharacter.h"

#include "AIController.h"
#include "EnemySubAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "PhysicsEngine/ConstraintInstance.h"

void ABomberCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABomberCharacter, bIsDiving);
	DOREPLIFETIME(ABomberCharacter, bIsExploding);
}

ABomberCharacter::ABomberCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ABomberCharacter::HandleDeath()
{
	AAIController* AIController = Cast<AAIController>(GetController());
	if (!AIController)
	{
		Super::HandleDeath();
		return;
	}
	UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		Super::HandleDeath();
		return;
	}
	BlackboardComponent->SetValueAsBool("IsGonnaExplode", true);
}

void ABomberCharacter::HandleHit(struct FDamageEvent const& DamageEvent, AActor* DamageCauser)
{
	if (AController* ControllerNullCheck = GetController())
	{
		Super::HandleHit(DamageEvent, DamageCauser);
		ControllerNullCheck->StopMovement();
		if (DamageCauser)
		{
			LaunchCharacter((FVector(0.f, 0.f, 0.5f) + DamageCauser->GetActorForwardVector()) * 500 , false, false);
		}
		else
		{
			LaunchCharacter((FVector(0.f, 0.f, 1.f) * 655) , false, false);
		}
	}
}

void ABomberCharacter::Server_Explode_Implementation()
{
	Server_SpawnExplosion(GetActorLocation(),GetActorRotation());
	Super::HandleDeath();
}


void ABomberCharacter::Server_SpawnExplosion_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (!HasAuthority()) return;
	
	SetActorEnableCollision(false);
	AActor* Explosion = GetWorld()->SpawnActor<AActor>(ExplosionActor, GetActorLocation(),GetActorRotation());
	Cast<AEnemySubAttack>(Explosion)->DamageMultiplier = DamageMultiplier;
	

}


