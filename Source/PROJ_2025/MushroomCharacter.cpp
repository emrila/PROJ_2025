// Furkan approves of this


#include "MushroomCharacter.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AMushroomCharacter::AMushroomCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AMushroomCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMushroomCharacter, AttackIsOnCooldown);
	DOREPLIFETIME(AMushroomCharacter, bIsAttacking);
	DOREPLIFETIME(AMushroomCharacter, Health);
}

void AMushroomCharacter::Multicast_Jump_Implementation(float Angle, FRotator RotationToPlayer, float JumpStrength, float ForwardStrength)
{
	RotationToPlayer.Yaw += Angle;
	FVector JumpDir = RotationToPlayer.Vector().GetSafeNormal();
	FVector LaunchVelocity = JumpDir * ForwardStrength + FVector(0, 0, JumpStrength);

	SetActorRotation(RotationToPlayer);
	LaunchCharacter(LaunchVelocity, true, true);
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

float AMushroomCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	if (Health <= 0)
	{
		Destroy(true);
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

