#include "ShieldAttackComp.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/Shield.h"


UShieldAttackComp::UShieldAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmount = 10.f;
	AttackCooldown = 20.f;
	BaseDurability = 200.f;
	BaseRecoveryRate = 1.f;
}

void UShieldAttackComp::TickComponent(float DeltaTime, ELevelTick TickType,
									  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UShieldAttackComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &UShieldAttackComp::StartAttack);
	}
}

void UShieldAttackComp::StartAttack()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (!bCanAttack)
	{
		return;
	}
	
	if (!bIsShieldActive)
	{
		Server_ActivateShield();
		return;
	}

	Server_DeactivateShield();
}

void UShieldAttackComp::Server_SpawnShield_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !ShieldClass)
	{
		return;
	}

	FVector SpawnLoc = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 120.f;
	FRotator SpawnRot = OwnerCharacter->GetActorRotation();

	FActorSpawnParameters Params;
	Params.Owner = OwnerCharacter;
	Params.Instigator = OwnerCharacter->GetInstigator();

	// Spawn on server; the spawned actor must replicate (see AShield constructor)
	AShield* Shield = GetWorld()->SpawnActor<AShield>(ShieldClass, SpawnLoc, SpawnRot, Params);
	if (Shield)
	{
		// set server-side owner/initial values and store authoritative pointer
		Shield->SetOwnerCharacter(Cast<APlayerCharacterBase>(OwnerCharacter));
		// Ensure actor is set to replicate (extra safety)
		Shield->SetReplicates(true);
		Shield->SetReplicateMovement(true);

		CurrentShield = Shield;

		// default to deactivated state on spawn (server authoritative)
		Shield->DeactivateShield();
	}
}

void UShieldAttackComp::Multicast_SpawnShield_Implementation()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (ShieldClass)
	{
		FVector SpawnLoc = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 120.f;
		FRotator SpawnRot = OwnerCharacter->GetActorRotation();

		FActorSpawnParameters Params;
		Params.Owner = OwnerCharacter;

		AShield* Shield = GetWorld()->SpawnActor<AShield>(ShieldClass, SpawnLoc, SpawnRot, Params);
		if (Shield)
		{
			Shield->SetDamageAmount(GetDamageAmount());
			Shield->SetDurability(GetDurability());
			Shield->SetRecoveryRate(GetRecoveryRate());
			bIsShieldActive = true;
			if (APlayerCharacterBase* PlayerCharacterBase = Cast<APlayerCharacterBase>(OwnerCharacter); Shield)
			{
				Shield->SetOwnerCharacter(PlayerCharacterBase);
				CurrentShield = Shield;
			}
		}
	}
}

float UShieldAttackComp::GetAttackCooldown() const
{
	return Super::GetAttackCooldown() * AttackSpeedModifier;
}

float UShieldAttackComp::GetDamageAmount() const
{
	if (FMath::IsNearlyEqual(AttackDamageModifier, 1.f, 0.0001f)) //if (AttackDamageModifier == 1.f)
	{
		return Super::GetDamageAmount();
	}
	//5.f is random atm, TBD later
	return Super::GetDamageAmount() + (AttackDamageModifier * 5.f);
}

float UShieldAttackComp::GetDurability()
{
	if (AttackDamageModifier ==1.f)
	{
		return BaseDurability;
	}
	//50.f is random atm, TBD later
	return BaseDurability + (AttackDamageModifier * 50.f);
}

float UShieldAttackComp::GetRecoveryRate()
{
	return BaseRecoveryRate * AttackSpeedModifier;
}

void UShieldAttackComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, CurrentShield);
}

void UShieldAttackComp::Server_ActivateShield_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	Multicast_ActivateShield();
}

void UShieldAttackComp::Multicast_ActivateShield_Implementation()
{
	if (!OwnerCharacter || !CurrentShield)
	{
		return;
	}

	bIsShieldActive = true;
	
	CurrentShield->SetDamageAmount(GetDamageAmount());
	CurrentShield->SetDurability(GetDurability());
	CurrentShield->SetRecoveryRate(GetRecoveryRate());
	CurrentShield->ActivateShield();
}

void UShieldAttackComp::Server_DeactivateShield_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	Multicast_DeactivateShield();
}

void UShieldAttackComp::Multicast_DeactivateShield_Implementation()
{
	if (!OwnerCharacter || !CurrentShield)
	{
		return;
	}
	CurrentShield->DeactivateShield();
	bIsShieldActive = false;
}

void UShieldAttackComp::StartAttackCooldown()
{
	if (!OwnerCharacter || !bCanAttack)
	{
		return;
	}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UShieldAttackComp::ResetAttackCooldown, GetAttackCooldown(), false);
}

void UShieldAttackComp::BeginPlay()
{
	Super::BeginPlay();
	Server_SpawnShield();

	if (CurrentShield)
	{
		Server_DeactivateShield();
	}
}


