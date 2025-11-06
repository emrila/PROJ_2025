

#include "AttackComponent.h"

#include "MageProjectile.h"
#include "GameFramework/Character.h"


UAttackComponent::UAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttackComponent::StartAttack()
{
	if (!bCanAttack)
	{
		return;
	}

	if (!OwnerCharacter || !ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackComponent, OwnerCharacter or ProjectileClass is NULL!"));
		return;
	}

	bCanAttack = false;
	
	PerformAttack();

	GetWorld()->GetTimerManager().SetTimer(
		AttackCoolDownTimerHandle,
		this,
		&UAttackComponent::ResetAttackCooldown,
		AttackCoolDown,
		false
		);
}


void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());

	//check(OwnerCharacter != nullptr);
	
}

void UAttackComponent::Server_SpawnProjectile_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
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

	UWorld* const World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComp, SpawnProjectile, World is NULL"));
		return;
	}
	
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	
	
	World->SpawnActor<AMageProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParameters))
	
}

bool UAttackComponent::Server_SpawnProjectile_Validate(FVector SpawnLocation, FRotator SpawnRotation)
{
	return !SpawnLocation.IsZero() && !SpawnRotation.IsZero();
}

void UAttackComponent::PerformAttack()
{
	if (!OwnerCharacter || !ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComp, SpawnProjectile, OwnerCharacter || ProjectileClass is NULL!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("PerformAttack"));

	//TODO: Handle Animation from here?

	FTransform SpawnTransform = GetProjectileTransform();
	
	Server_SpawnProjectile(SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());
}

FTransform UAttackComponent::GetProjectileTransform()
{
	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (OwnerCharacter)
	{
		USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();

		if (MeshComp && MeshComp->DoesSocketExist(ProjectileSpawnSocketName))
		{
			UE_LOG(LogTemp, Warning, TEXT("Socket exists"));
			SpawnLocation = MeshComp->GetSocketLocation(ProjectileSpawnSocketName);
			SpawnRotation = MeshComp->GetSocketRotation(ProjectileSpawnSocketName);
		}
		else  //Fallback if the socket is not found or does not exist
		{
			SpawnLocation = OwnerCharacter->GetActorLocation();
			SpawnRotation = OwnerCharacter->GetActorRotation();
			
			FVector EyeLocation;
			FRotator EyeRotation;
			
			OwnerCharacter->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			SpawnLocation = EyeLocation + EyeRotation.Vector() * 80.f;
			SpawnRotation = EyeRotation;
		}
	}

	SpawnLocation += SpawnRotation.RotateVector(SpawnLocationOffset);
	
	return FTransform(SpawnRotation, SpawnLocation);
}

void UAttackComponent::ResetAttackCooldown()
{
	bCanAttack = true;
}

