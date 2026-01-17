#include "RangeAttackComp.h"

#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/MageProjectile.h"


URangeAttackComp::URangeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	
	DamageAmount = 15.f;
	AttackCooldown = 0.5f;
}

float URangeAttackComp::GetProjectileSpeed() const
{
	if (FMath::IsNearlyEqual(AttackSpeedModifier, 1.f, 0.001f))
	{
		return DefaultProjectileSpeed;
	}
	
	float const NewSpeedMultiplier = 2.f - AttackSpeedModifier;
	return DefaultProjectileSpeed * NewSpeedMultiplier;
}

void URangeAttackComp::StartAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	
	if (!ProjectileClass)
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("%s, ProjectileClass is NULL!"), *FString(__FUNCTION__));
		return;
	}
	
	if (!bCanAttack || !OwnerCharacter->IsAlive())
	{
		return;
	}
	
	if (OwnerCharacter->IsAttacking())
	{
		return;
	}
	Super::StartAttack();
	
	PerformAttack();
}

void URangeAttackComp::PerformAttack()
{
	Super::PerformAttack();
	RequestSpawnProjectile();
}

void URangeAttackComp::RequestSpawnProjectile()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	if (OwnerCharacter->HasAuthority())
	{
		SpawnProjectile(GetProjectileTransform());
	}
	else
	{
		Server_SpawnProjectile(GetProjectileTransform());
	}
}

void URangeAttackComp::Server_SpawnProjectile_Implementation(const FTransform& SpawnTransform)
{
	SpawnProjectile(SpawnTransform);
}

void URangeAttackComp::SpawnProjectile(const FTransform& SpawnTransform) const
{
	if (!OwnerCharacter || !ProjectileClass)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("AttackComp, SpawnProjectile, OwnerCharacter || ProjectileClass is NULL!"));
		return;
	}
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	
	if (SpawnTransform.GetLocation().IsNearlyZero())
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("%s, SpawnTransform is zero! no projectiles will be spawned"), *FString(__FUNCTION__));
		return;
	}
	
	AMageProjectile* Projectile = GetWorld()->SpawnActorDeferred<AMageProjectile>(
		ProjectileClass, 
		SpawnTransform, 
		OwnerCharacter, 
		OwnerCharacter, 
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	
	if (Projectile)
	{
		Projectile->SetDamageAmount(GetDamageAmount());
		Projectile->SetProjectileSpeed(GetProjectileSpeed());
		
		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);
	}
}

FTransform URangeAttackComp::GetProjectileTransform() const
{
	if (!OwnerCharacter)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return FTransform::Identity;
	}

	if (!OwnerCharacter->IsLocallyControlled())
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("%s Pawn is not locally controlled; skipping camera-based locking."),
			   *FString(__FUNCTION__));
		return FTransform::Identity;
	}

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s PlayerController is Null."), *FString(__FUNCTION__));
		return FTransform::Identity;
	}

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;

	if (!CameraManager)
	{
		UE_LOG(AttackComponentLog, Error, TEXT("%s CameraManager is Null."), *FString(__FUNCTION__));
		return FTransform::Identity;
	}

	FVector CameraLocation = CameraManager->GetCameraLocation() + (CameraManager->GetActorForwardVector() * ProjectileOffsetDistanceInFront);
	FRotator CameraRotation = CameraManager->GetCameraRotation();
	
	return FTransform(CameraRotation, CameraLocation);
}




