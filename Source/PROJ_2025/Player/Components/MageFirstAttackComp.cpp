#include "MageFirstAttackComp.h"
#include "MageProjectile.h"
#include "GameFramework/Character.h"
#include "Player/Characters/PlayerCharacterMage.h"


UMageFirstAttackComp::UMageFirstAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMageFirstAttackComp::StartAttack()
{
	if (!bCanAttack)
	{
		return;
	}
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
	//const FVector SpawnLocation = GetProjectileSpawnLocation();

	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, PerformAttack, OwnerCharacter is NULL!"));
		return;
	}
	
	const FTransform SpawnTransform = GetProjectileTransform();
	
	Server_SpawnProjectile(SpawnTransform);
}

void UMageFirstAttackComp::Server_SpawnProjectile_Implementation(const FTransform SpawnTransform)
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
	
	GetWorld()->SpawnActor<AMageProjectile>(ProjectileClass, SpawnTransform, SpawnParameters);
}

FTransform UMageFirstAttackComp::GetProjectileTransform()
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
	const FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	const FVector OwnerForwardVector = OwnerCharacter->GetActorForwardVector();
	const FVector OwnerCharacterRightVector = OwnerCharacter->GetActorRightVector();
	FVector SpawnLocation = (OwnerLocation + OwnerCharacterRightVector * 80) + OwnerForwardVector ;
	FRotator SpawnRotation = GetProjectileSpawnRotation();
	
	return FTransform(SpawnRotation, SpawnLocation);
}

FRotator UMageFirstAttackComp::GetProjectileSpawnRotation()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMageAttackComp::GetProjectileSpawnRotation, OwnerCharacter is NULL"));
		return FRotator::ZeroRotator;
	}

	APlayerController* PlayerController = OwnerCharacter->GetLocalViewingPlayerController();

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AMageAttackComp::GetProjectileSpawnRotation, PlayerController is NULL"));
		return FRotator::ZeroRotator;
	}

	int32 ViewPortX, ViewPortY;

	PlayerController->GetViewportSize(ViewPortX, ViewPortY);

	FVector2D ScreenCenter(ViewPortX / 2, ViewPortY / 2);

	FVector WorldLocation, WorldDirection;

	if (PlayerController->DeprojectScreenPositionToWorld(
		ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection))
	{
		return WorldDirection.Rotation();
	}

	UE_LOG(LogTemp, Warning, TEXT("AMageAttackComp::GetProjectileSpawnRotation, Unable to get screen center rotation"));

	return FRotator::ZeroRotator;
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

