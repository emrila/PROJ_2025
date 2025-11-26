// Furkan approves of this


#include "MushroomCharacter.h"

#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
#include "Player/Components/Items/ChronoRiftDamageType.h"

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

void AMushroomCharacter::StartSmoothRotationTo(FRotator TargetRotation, float Speed)
{
	DesiredRotation = TargetRotation;
	RotationSpeed = Speed;
	
	GetWorldTimerManager().ClearTimer(RotationTimerHandle);
	
	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&AMushroomCharacter::UpdateSmoothRotation,
		0.01f,      
		true
	);
}

void AMushroomCharacter::UpdateSmoothRotation()
{
	FRotator Current = GetActorRotation();

	FRotator NewRot =
		FMath::RInterpTo(Current, DesiredRotation, 0.01f, RotationSpeed);

	SetActorRotation(NewRot);

	if (Current.Equals(DesiredRotation, 0.5f))
	{
		GetWorldTimerManager().ClearTimer(RotationTimerHandle);
	}
}

void AMushroomCharacter::HandleHit(struct FDamageEvent const& DamageEvent, AActor* DamageCauser)
{
	if (AController* ControllerNullCheck = GetController())
	{
		Super::HandleHit(DamageEvent, DamageCauser);
		ControllerNullCheck->StopMovement();
		
		if (DamageEvent.DamageTypeClass == UChronoRiftDamageType::StaticClass())
		{
			return;
		}
		if (DamageCauser)
		{
			LaunchCharacter(DamageCauser->GetActorForwardVector() * 1555, false, false);
		}else
		{
			LaunchCharacter((FVector(0.f, 0.f, 1.f) * 655) , false, false);
		}
	}
}

void AMushroomCharacter::Multicast_Jump_Implementation(float Angle, FRotator RotationToPlayer, float JumpStrength, float ForwardStrength)
{
	RotationToPlayer.Yaw += Angle;
	FVector JumpDir = RotationToPlayer.Vector().GetSafeNormal();
	FVector LaunchVelocity = JumpDir * ForwardStrength + FVector(0, 0, JumpStrength);
	
	StartSmoothRotationTo(RotationToPlayer, RotateSpeed);
	LaunchCharacter(LaunchVelocity, true, true);
}





