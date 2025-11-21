// Furkan approves of this


#include "CactusCharacter.h"

#include "AIController.h"
#include "EnemySubAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"

void ACactusCharacter::Server_ShootProjectile_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation);
	Cast<AEnemySubAttack>(Projectile)->DamageMultiplier = DamageMultiplier;

}

void ACactusCharacter::Server_SpawnSpikeExplosion_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (!HasAuthority()) return;
	
	AActor* Explosion = GetWorld()->SpawnActor<AActor>(SpikeExplosionClass, GetActorLocation(),GetActorRotation());
	Cast<AEnemySubAttack>(Explosion)->DamageMultiplier = DamageMultiplier;
}

void ACactusCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACactusCharacter, bIsBurrowing);
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
			FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaSeconds, InterpSpeed);
			SetActorRotation(NewRot);
		}
	}
}
