#include "MageFirstAttackComp.h"
#include "MageProjectile.h"
#include "GameFramework/Character.h"


UMageFirstAttackComp::UMageFirstAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMageFirstAttackComp::StartAttack()
{
	Super::StartAttack();
	
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MageFirstAttackComp, StartAttack, ProjectileClass is NULL!"));
		return;
	}

	PerformAttack();
}

void UMageFirstAttackComp::BeginPlay()
{
	Super::BeginPlay();
}

void UMageFirstAttackComp::PerformAttack()
{
	//Super::PerformAttack();
	//const FTransform SpawnTransform = GetProjectileTransform();

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, PerformAttack, OwnerCharacter is NULL!"));
		return;
	}

	const FVector SpawnLocation = GetProjectileSpawnLocation();
	
	Server_SpawnProjectile(SpawnLocation);
}

void UMageFirstAttackComp::Server_SpawnProjectile_Implementation(const FVector SpawnLocation)
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
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	
	GetWorld()->SpawnActor<AMageProjectile>(ProjectileClass, SpawnLocation, OwnerCharacter->GetControlRotation(), SpawnParameters);
}

FTransform UMageFirstAttackComp::GetProjectileTransform()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, GetProjectileTransform, OwnerCharacter is NULL!"));
		return FTransform::Identity;
	}
	
	FVector SpawnLocation;
	FRotator SpawnRotation;

	if (
		const USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
		MeshComp && MeshComp->DoesSocketExist(ProjectileSpawnSocketName)
		)
	{
		SpawnLocation = MeshComp->GetSocketLocation(ProjectileSpawnSocketName);
		SpawnRotation = MeshComp->GetSocketRotation(ProjectileSpawnSocketName);
	}

	SpawnLocation += SpawnRotation.RotateVector(SpawnLocationOffset);
	
	return FTransform(SpawnRotation, SpawnLocation);
}

FVector UMageFirstAttackComp::GetProjectileSpawnLocation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, GetProjectileSpawnLocation, OwnerCharacter is NULL!"));
		return FVector::ZeroVector;
	}
	
	const USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();

	if (!MeshComp || !MeshComp->DoesSocketExist(ProjectileSpawnSocketName))
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, GetProjectileSpawnLocation, MeshComp is NULL or Socket does not exist!"));
		return FVector::ZeroVector;
	}
	
	return MeshComp->GetSocketLocation(ProjectileSpawnSocketName);
}

