// Furkan approves of this


#include "BomberCharacter.h"

#include "AIController.h"
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
	AAIController* Controller = Cast<AAIController>(GetController());
	if (!Controller)
	{
		return;
	}
	UBlackboardComponent* BlackboardComponent = Controller->GetBlackboardComponent();
	if (!BlackboardComponent)
	{
		return;
	}
	BlackboardComponent->SetValueAsBool("IsGonnaExplode", true);
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
	GetWorld()->SpawnActor<AActor>(ExplosionActor, GetActorLocation(),GetActorRotation());

}


