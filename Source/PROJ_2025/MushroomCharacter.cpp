// Furkan approves of this


#include "MushroomCharacter.h"

#include "Net/UnrealNetwork.h"

AMushroomCharacter::AMushroomCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AMushroomCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMushroomCharacter, AttackIsOnCooldown);
	DOREPLIFETIME(AMushroomCharacter, bIsAttacking);
	DOREPLIFETIME(AMushroomCharacter, bIsRunning);
	DOREPLIFETIME(AMushroomCharacter, Bounce);
	DOREPLIFETIME(AMushroomCharacter, SlowRunning);
	DOREPLIFETIME(AMushroomCharacter, WarmingUp);
	DOREPLIFETIME(AMushroomCharacter, MeleeRun);
	DOREPLIFETIME(AMushroomCharacter, Projectile);
}

void AMushroomCharacter::HandleHit()
{
	Super::HandleHit();
	LaunchCharacter(GetActorForwardVector() * -1555, false, false);
}

void AMushroomCharacter::Multicast_Jump_Implementation(float Angle, FRotator RotationToPlayer, float JumpStrength, float ForwardStrength)
{
	RotationToPlayer.Yaw += Angle;
	FVector JumpDir = RotationToPlayer.Vector().GetSafeNormal();
	FVector LaunchVelocity = JumpDir * ForwardStrength + FVector(0, 0, JumpStrength);

	SetActorRotation(RotationToPlayer);
	LaunchCharacter(LaunchVelocity, true, true);
}





