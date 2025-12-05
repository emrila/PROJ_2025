#include "RangeAttackComp.h"

#include "EnhancedInputComponent.h"
#include "../Items/MageProjectile.h"
#include "GameFramework/Character.h"
#include "Player/Characters/PlayerCharacterBase.h"


URangeAttackComp::URangeAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmountToStore = 10.f;
	AttackCooldown = 0.5f;
}

void URangeAttackComp::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsAttacking)
	{
		StartAttack();
	}
}

void URangeAttackComp::StartAttack()
{
	if (!bCanAttack)
	{
		UE_LOG(LogTemp, Warning, TEXT("Starting attack."));
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

void URangeAttackComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &URangeAttackComp::OnStartAttack);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this, &URangeAttackComp::OnAttackEnd);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Canceled, this, &URangeAttackComp::OnAttackCanceled);
	}
}

void URangeAttackComp::BeginPlay()
{
	Super::BeginPlay();
}

void URangeAttackComp::OnStartAttack(const FInputActionInstance& Instance)
{
	if (Instance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%s, Attacking."), *FString(__FUNCTION__));

	bIsAttacking = true;
}

void URangeAttackComp::OnAttackEnd(const FInputActionInstance& Instance)
{
	if (Instance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}
	if (!bIsAttacking)
	{
		return;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%s, Completing attack."), *FString(__FUNCTION__));
	bIsAttacking = false;
}

void URangeAttackComp::OnAttackCanceled(const FInputActionInstance& Instance)
{
	if (Instance.GetTriggerEvent() != ETriggerEvent::Canceled)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("%s, Canceling attack."), *FString(__FUNCTION__));
	bIsAttacking = false;
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
