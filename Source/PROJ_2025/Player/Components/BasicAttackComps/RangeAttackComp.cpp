#include "RangeAttackComp.h"

#include "EnhancedInputComponent.h"
#include "../Items/MageProjectile.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
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
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is NULL!"), *FString(__FUNCTION__));
		return;
	}
	
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("MageFirstAttackComp, StartAttack, ProjectileClass is NULL!"));
		return;
	}
	
	if (!bCanAttack || !OwnerCharacter->IsAlive())
	{
		return;
	}
	
	Super::StartAttack();
	PerformAttack();
}

void URangeAttackComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &URangeAttackComp::OnStartAttack);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this, &URangeAttackComp::OnAttackEnd);
	}
}

void URangeAttackComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(URangeAttackComp, ProjectileSpawnTransform);
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
	bIsAttacking = false;
}

void URangeAttackComp::PerformAttack()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("MageFirstAttackComp, PerformAttack, OwnerCharacter is NULL!"));
		return;
	}
	
	PlayAttackAnim();
	
	const FTransform SpawnTransform = GetProjectileTransform();
	
	SpawnProjectile(SpawnTransform);
}

void URangeAttackComp::SpawnProjectile(const FTransform SpawnTransform)
{
	if (!OwnerCharacter || !ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComp, SpawnProjectile, OwnerCharacter || ProjectileClass is NULL!"));
		return;
	}
	if (OwnerCharacter->HasAuthority())
	{
		ProjectileSpawnTransform = SpawnTransform;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = OwnerCharacter;
		SpawnParameters.Instigator = OwnerCharacter;
		AMageProjectile* Projectile = GetWorld()->SpawnActor<AMageProjectile>(
			ProjectileClass, SpawnTransform, SpawnParameters);
	
		if (Projectile)
		{
			Projectile->Server_SetDamageAmount(GetDamageAmount());
			Projectile->SetImpactParticle(OwnerCharacter->ImpactParticles);
		}
	}
	else
	{
		Server_SpawnProjectile(SpawnTransform);
	}
}

void URangeAttackComp::Server_SpawnProjectile_Implementation(const FTransform SpawnTransform)
{
	if (!OwnerCharacter || !ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("AttackComp, SpawnProjectile, OwnerCharacter || ProjectileClass is NULL!"));
		return;
	}
	
	ProjectileSpawnTransform = SpawnTransform;
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = OwnerCharacter;
	SpawnParameters.Instigator = OwnerCharacter;
	AMageProjectile* Projectile = GetWorld()->SpawnActor<AMageProjectile>(
		ProjectileClass, SpawnTransform, SpawnParameters);
	
	if (Projectile)
	{
		Projectile->Server_SetDamageAmount(GetDamageAmount());
		Projectile->SetImpactParticle(OwnerCharacter->ImpactParticles);
	}
}

void URangeAttackComp::PlayAttackAnim()
{
	if (OwnerCharacter && OwnerCharacter->HasAuthority())
	{
		Multicast_PlayAttackAnim();
	}
	else
	{
		Server_PlayAttackAnim();
	}
}

void URangeAttackComp::Server_PlayAttackAnim_Implementation()
{
	Multicast_PlayAttackAnim();
}

void URangeAttackComp::Multicast_PlayAttackAnim_Implementation()
{
	if (AttackAnimation && OwnerCharacter)
	{
		float PlayRate = 1.f;
		if (AttackAnimation->GetPlayLength() > GetAttackCooldown())
		{
			const float AnimLength = AttackAnimation->GetPlayLength();
			PlayRate = AnimLength / GetAttackCooldown();
		}
		OwnerCharacter->PlayAnimMontage(AttackAnimation, PlayRate);
	}
}

FTransform URangeAttackComp::GetProjectileTransform()
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s OwnerCharacter is Null."), *FString(__FUNCTION__));
		return FTransform::Identity;
	}

	if (!OwnerCharacter->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Pawn is not locally controlled; skipping camera-based locking."),
			   *FString(__FUNCTION__));
		return FTransform::Identity;
	}

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
