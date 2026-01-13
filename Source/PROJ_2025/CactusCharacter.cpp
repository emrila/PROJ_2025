// Furkan approves of this


#include "CactusCharacter.h"

#include "AIController.h"
#include "EnemySubAttack.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

void ACactusCharacter::Server_ShootProjectile_Implementation(const FVector& NewTargetLocation)
{
	if (!HasAuthority()) return;
	
	TargetActorLocation = NewTargetLocation;
	
	if (!bIsPlayingAnimation)
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(InitialAttackTimer))
		{
			GetWorld()->GetTimerManager().SetTimer(InitialAttackTimer, [this] ()
			{
				Multicast_PlayAttackMontage();
			}, 0.5f, false);
		}
		if (!GetWorld()->GetTimerManager().IsTimerActive(AnimationTimer))
		{
			float Delay = 1.5f;
			if (AttackAnim)
			{
				Delay = AttackAnim->GetPlayLength() / 2.f + 0.5f;
			}
			GetWorld()->GetTimerManager().SetTimer(AnimationTimer, [this] ()
			{
				bIsPlayingAnimation = false;
			}, Delay, false);
		}
	}
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
	
	if (TargetActorLocation.IsNearlyZero())
	{
		return;
	}
	
	ProjectileSpawnRotation = UKismetMathLibrary::FindLookAtRotation(CurrentProjectileSocketLocation, TargetActorLocation);
	ForceNetUpdate();
	
	if (ProjectileSpawnRotation.IsNearlyZero())
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
		PlayAnimMontage(AttackAnim, 2.f);
		bIsPlayingAnimation = true;
	}
}

void ACactusCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	OnAttackAnimNotify.AddDynamic(this, &ACactusCharacter::Server_HandleOnAttackAnimNotify);
}

void ACactusCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACactusCharacter, bIsBurrowing);
	DOREPLIFETIME(ACactusCharacter, CurrentProjectileSocketLocation);
	DOREPLIFETIME(ACactusCharacter, bIsPlayingAnimation);
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

float ACactusCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,class AController* EventInstigator, AActor* DamageCauser)
{
	if (!bIsBurrowing)
	{
		return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	}
	return 0;
}
