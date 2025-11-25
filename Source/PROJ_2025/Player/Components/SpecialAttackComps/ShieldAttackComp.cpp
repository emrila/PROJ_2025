#include "ShieldAttackComp.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Shield.h"


UShieldAttackComp::UShieldAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmount = 10.f;
	AttackCooldown = 20.f;
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
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &UShieldAttackComp::OnStartAttack);
	}
}

void UShieldAttackComp::StartAttack()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (!bCanAttack || bIsShieldActive)
	{
		return;
	}

	TheCurrentDurability = GetCurrentDurability();
	Multicast_SpawnShield();

	GetWorld()->GetTimerManager().ClearTimer(DurabilityTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		DurabilityTimerHandle,
		[this]()
		{
			if (TheCurrentDurability <= 0.f)
			{
				if (CurrentShield)
				{
					CurrentShield->Destroy();
					CurrentShield = nullptr;
				}
				bIsShieldActive = false;
				GetWorld()->GetTimerManager().ClearTimer(DurabilityTimerHandle);
				GetWorld()->GetTimerManager().SetTimer(RecoveryTimerHandle, this, &UShieldAttackComp::RecoverDurability, GetRecoveryRate(), true);
				Super::StartAttack();
			}
			else
			{
				TheCurrentDurability -= 10.f;
			}
		},
		1.f,
		true
		);
	
}

void UShieldAttackComp::PerformAttack()
{
	Super::PerformAttack();
}

void UShieldAttackComp::OnStartAttack(const FInputActionInstance& ActionInstance)
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (!bCanAttack)
	{
		return;
	}
	
	if (ActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}
	StartAttack();
}

void UShieldAttackComp::Multicast_SpawnShield_Implementation()
{
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
		}
		if (APlayerCharacterBase* PlayerCharacterBase = Cast<APlayerCharacterBase>(OwnerCharacter); Shield)
		{
			Shield->SetOwnerCharacter(PlayerCharacterBase);
		}
			

		CurrentShield = Shield;
		bIsShieldActive = true;
	}
}

void UShieldAttackComp::RecoverDurability()
{
	if (TheCurrentDurability >= GetCurrentDurability())
	{
		TheCurrentDurability = 0.f;
		if (GetWorld()->GetTimerManager().IsTimerActive(RecoveryTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
		}
		return;
	}
	
	TheCurrentDurability += 10.f;
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

float UShieldAttackComp::GetCurrentDurability()
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
	return BaseRecoveryRate / AttackSpeedModifier;
}

void UShieldAttackComp::BeginPlay()
{
	Super::BeginPlay();
	
}
