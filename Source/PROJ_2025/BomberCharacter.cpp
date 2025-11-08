// Furkan approves of this


#include "BomberCharacter.h"

#include "Net/UnrealNetwork.h"

void ABomberCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABomberCharacter, bIsDiving);
}

ABomberCharacter::ABomberCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ABomberCharacter::HandleDeath()
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


