// Furkan approves of this


#include "MushroomCharacter.h"

// Sets default values
AMushroomCharacter::AMushroomCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AMushroomCharacter::Multicast_Jump_Implementation(float Angle, FRotator RotationToPlayer, float JumpStrength, float ForwardStrength)
{
	
}

// Called when the game starts or when spawned
void AMushroomCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMushroomCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMushroomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

