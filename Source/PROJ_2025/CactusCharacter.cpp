// Furkan approves of this


#include "CactusCharacter.h"

#include "AIController.h"
#include "EnemySubAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

void ACactusCharacter::Server_ShootProjectile_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (!HasAuthority()) return;
	
	Multicast_PlayAttackMontage();
}

void ACactusCharacter::Server_SpawnSpikeExplosion_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (!HasAuthority()) return;
	
	AActor* Explosion = GetWorld()->SpawnActor<AActor>(SpikeExplosionClass, GetActorLocation(),GetActorRotation());
	Cast<AEnemySubAttack>(Explosion)->DamageMultiplier = DamageMultiplier;
}

void ACactusCharacter::Server_HandleOnAttackAnimNotify_Implementation(const FVector SpawnLocation)
{
	if (!HasAuthority()) return;
	CurrentProjectileSocketLocation = SpawnLocation;
	ForceNetUpdate();
	
	if (CurrentProjectileSocketLocation.IsNearlyZero())
	{
		return;
	}
	
	
	AEnemySubAttack* Projectile = Cast<AEnemySubAttack>(
	UGameplayStatics::BeginDeferredActorSpawnFromClass(
		this,
		ProjectileClass,
		FTransform(ProjectileSpawnRotation, CurrentProjectileSocketLocation),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		)
	);
	if (Projectile)
	{
		Projectile->DamageMultiplier = DamageMultiplier;

		UGameplayStatics::FinishSpawningActor(
			Projectile,
			FTransform(ProjectileSpawnRotation, CurrentProjectileSocketLocation)
		);
	}
}

void ACactusCharacter::Multicast_PlayAttackMontage_Implementation()
{
	if (AttackAnim)
	{
		PlayAnimMontage(AttackAnim);
	}
}


void ACactusCharacter::Server_SetProjectileSpawnRotation_Implementation(const FVector& NewTargetActorLocation)
{
	if (!HasAuthority()) return;
	if (!CurrentProjectileSocketLocation.IsNearlyZero())
	{
		ProjectileSpawnRotation = UKismetMathLibrary::FindLookAtRotation(CurrentProjectileSocketLocation, NewTargetActorLocation);
		ForceNetUpdate();
	}
}

void ACactusCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	OnAttackAnimNotify.AddDynamic(this, &ACactusCharacter::Server_HandleOnAttackAnimNotify);
	
	/*if (GetMesh())
	{
		if (GetMesh()->DoesSocketExist(FirestProjectileSocketName))
		{
			FirstProjectileSocketLocation = GetMesh()->GetSocketLocation(FirestProjectileSocketName);
		}
		if (GetMesh()->DoesSocketExist(SecondProjectileSocketName))
		{
			SecondProjectileSocketLocation = GetMesh()->GetSocketLocation(SecondProjectileSocketName);
		}
		
		if (HasAuthority())
		{
			bUsingFirstProjectileSocket = true;
			CurrentProjectileSocketLocation = FirstProjectileSocketLocation;
		}
		else
		{
			CurrentProjectileSocketLocation = FirstProjectileSocketLocation;
		}
	}*/
}

void ACactusCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACactusCharacter, bIsBurrowing);
	DOREPLIFETIME(ACactusCharacter, FirstProjectileSocketLocation);
	DOREPLIFETIME(ACactusCharacter, SecondProjectileSocketLocation);
	DOREPLIFETIME(ACactusCharacter, CurrentProjectileSocketLocation);
	DOREPLIFETIME(ACactusCharacter, bUsingFirstProjectileSocket);
	DOREPLIFETIME(ACactusCharacter, ProjectileSpawnRotation);
	DOREPLIFETIME(ACactusCharacter, TargetActorLocation);
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
