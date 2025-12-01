#include "RangeAttackComp.h"
#include "../Items/MageProjectile.h"
#include "GameFramework/Character.h"
#include "Player/Characters/PlayerCharacterBase.h"


URangeAttackComp::URangeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmountToStore = 10.f;
	AttackCooldown = 0.5f;
}

void URangeAttackComp::StartAttack()
{
	if (!bCanAttack)
	{
		return;
	}
	Super::StartAttack();
	
	if (!Cast<APlayerCharacterBase>(OwnerCharacter)->IsAlive())
	{
		return;
	}
	
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MageFirstAttackComp, StartAttack, ProjectileClass is NULL!"));
		return;
	}

	PerformAttack();
}

void URangeAttackComp::BeginPlay()
{
	Super::BeginPlay();
}

void URangeAttackComp::PerformAttack()
{
	//Super::PerformAttack();
	//const FVector SpawnLocation = GetProjectileSpawnLocation();

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, PerformAttack, OwnerCharacter is NULL!"));
		return;
	}
	
	const FTransform SpawnTransform = GetProjectileTransform();
	
	Server_SpawnProjectile(SpawnTransform);
}

void URangeAttackComp::Server_SpawnProjectile_Implementation(const FTransform SpawnTransform)
{
	if (!OwnerCharacter || !ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComp, SpawnProjectile, OwnerCharacter || ProjectileClass is NULL!"));
		return;
	}

	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	Multicast_SpawnProjectile(SpawnTransform);
}

void URangeAttackComp::Multicast_SpawnProjectile_Implementation(const FTransform SpawnTransform)
{
	if (!OwnerCharacter || !ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComp, SpawnProjectile, OwnerCharacter || ProjectileClass is NULL!"));
		return;
	}
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	AMageProjectile* Projectile = GetWorld()->SpawnActor<AMageProjectile>(
		ProjectileClass, SpawnTransform, SpawnParameters);
	
	if (!Projectile)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComp, SpawnProjectile, Projectile is NULL!"));
		return;
	}
	
	Projectile->SetOwner(OwnerCharacter);
	Projectile->Server_SetDamageAmount(GetDamageAmount());
	
	if (AttackAnimation && OwnerCharacter)
	{
		OwnerCharacter->PlayAnimMontage(AttackAnimation);
	}
	
	APlayerCharacterBase* PlayerCharacter= Cast<APlayerCharacterBase>(OwnerCharacter);
	if (!PlayerCharacter || !PlayerCharacter->ImpactParticles)
	{
		UE_LOG(LogTemp, Error, TEXT("%s , PlayerCharacter or ImpactParticles is NULL!"), *FString(__FUNCTION__));
		return;
	}
	Projectile->SetImpactParticle(Cast<APlayerCharacterBase>(GetOwner())->ImpactParticles);
}

FTransform URangeAttackComp::GetProjectileTransform()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, GetProjectileSpawnTransform, OwnerCharacter is NULL!"));
		return FTransform::Identity;
	}

	/*const USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();

	if (!MeshComp || !MeshComp->DoesSocketExist(ProjectileSpawnSocketName))
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, GetProjectileSpawnTransform, MeshComp is NULL or Socket does not exist!"));
		return FTransform::Identity;
	}*/
	
	//FVector SpawnLocation = MeshComp->GetSocketLocation(ProjectileSpawnSocketName);
	//FRotator SpawnRotation = MeshComp->GetSocketRotation(ProjectileSpawnSocketName);
	/*const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	const FVector OwnerForwardVector = OwnerCharacter->GetActorForwardVector();
	const FVector OwnerCharacterRightVector = OwnerCharacter->GetActorRightVector();

	FVector SpawnLocation;
	FRotator SpawnRotation = GetProjectileSpawnRotation();*/
	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());

	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("%s PlayerController is Null."), *FString(__FUNCTION__));
		return FTransform::Identity;
	}

	APlayerCameraManager* CameraManager = PC->PlayerCameraManager;

	if (!CameraManager)
	{
		UE_LOG(LogTemp, Error, TEXT("%s CameraManager is Null."), *FString(__FUNCTION__));
		return FTransform::Identity;
	}

	FVector CameraLocation = CameraManager->GetCameraLocation() + (CameraManager->GetActorForwardVector() * ProjectileOffsetDistanceInFront);
	FRotator CameraRotation = CameraManager->GetCameraRotation();
	
	
	return FTransform(CameraRotation, CameraLocation);
}

float URangeAttackComp::GetAttackCooldown() const
{
	return Super::GetAttackCooldown() * AttackSpeedModifier;
}

float URangeAttackComp::GetDamageAmount() const
{
	return Super::GetDamageAmount() * AttackDamageModifier;
}
