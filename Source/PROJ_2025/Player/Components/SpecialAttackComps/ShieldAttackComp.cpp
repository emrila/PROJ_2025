#include "ShieldAttackComp.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/Shield.h"

UShieldAttackComp::UShieldAttackComp()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	DamageAmount = 10;
	AttackCooldown = 5.f;
}

void UShieldAttackComp::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp,
                                               UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &UShieldAttackComp::StartAttack);
	}
}

void UShieldAttackComp::HandleCooldown()
{
	bCanAttack = false;
	bIsShieldActive = false;
	
	HandleOwnerMovement(false);
	
	PlayShieldBrokenAnimation();
	
	if (OwnerCharacter)
	{
		OwnerCharacter->RequestSetIsAttacking(false);
	}
	
	if (const float Delay = GetCooldownDuration(); Delay <= 0.f)
	{
		Reset();
	}
	else
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(CooldownTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UShieldAttackComp::Reset, Delay, false);
		}
	}
}

float UShieldAttackComp::GetDurability() const
{
	return BaseDurability * AttackDamageModifier;
}

void UShieldAttackComp::HandleOnDurabilityChanged(const float NewDurability) const
{
	OnDurabilityChanged.Broadcast(NewDurability, GetDurability());
	Client_HandleOnDurabilityChanged(NewDurability);
}

void UShieldAttackComp::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle InitialTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(InitialTimerHandle, [this]()
	{
		if (OwnerCharacter)
		{
			OwnerCharacter->OnPlayerDied.AddDynamic(this, &UShieldAttackComp::HandlePlayerDeath);
			TArray<AActor*> ShieldActors;
			OwnerCharacter->GetAllChildActors(ShieldActors);

			//Bind to player died event if needed

			for (AActor* Actor : ShieldActors)
			{
				if (AShield* FoundShield = Cast<AShield>(Actor))
				{
					Shield = FoundShield;
					Shield->SetOwnerProperties(OwnerCharacter, this, GetDurability());
					break;
				}
			}
		}
	}, 1.5f, false);
}

void UShieldAttackComp::StartAttack()
{
	if (!OwnerCharacter || !Shield || !bCanAttack || !OwnerCharacter->IsAlive())
	{
		return;
	}

	bIsShieldActive = !bIsShieldActive;
	RequestToggleShield(bIsShieldActive);
	OwnerCharacter->RequestSetIsAttacking(bIsShieldActive);
}

void UShieldAttackComp::RequestToggleShield(const bool bShouldActivate)
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (OwnerCharacter->HasAuthority())
	{
		ToggleShield(bShouldActivate);
	}
	else
	{
		Server_ToggleShield(bShouldActivate);
	}
}

void UShieldAttackComp::ToggleShield(const bool bShouldActivate)
{
	if (!Shield)
	{
		return;
	}
	HandleOwnerMovement(bShouldActivate);
	if (bShouldActivate)
	{
		Shield->SetValuesPreActivation(GetDamageAmount(), GetRecoveryRate());
		Shield->ActivateShield();
		//PlayAnimation(ActivateShieldAnimation);
		RequestDebug();
	}
	else
	{
		Shield->DeactivateShield();
		//PlayAnimation(DeactivateShieldAnimation);
	}
}

void UShieldAttackComp::Server_ToggleShield_Implementation(const bool bShouldActivate)
{
	ToggleShield(bShouldActivate);
}

float UShieldAttackComp::GetRecoveryRate() const
{
	if (FMath::IsNearlyEqual(AttackSpeedModifier, 0.1f, 0.001f))
	{
		return BaseRecoveryRate * 0.1f;
	}
	return BaseRecoveryRate * AttackSpeedModifier;
}

void UShieldAttackComp::HandlePlayerDeath(const bool bNewIsAlive)
{
	if (!bNewIsAlive)
	{
		RequestToggleShield(false);
	}
}

void UShieldAttackComp::HandleOwnerMovement(const bool bShouldSlowDown)
{
	if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
	{
		if (bShouldSlowDown)
		{
			MovementSpeedToStore = OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;
			OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed *= OwnerMovementSpeedMultiplier;
			Client_HandleOwnerMovement(OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed);
		}
		else
		{
			OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = MovementSpeedToStore;
			Client_HandleOwnerMovement(OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed);
		}
	}
}

void UShieldAttackComp::Client_HandleOwnerMovement_Implementation(const float NewMoveSpeed)
{
	if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = NewMoveSpeed;
	}
}

void UShieldAttackComp::Client_HandleOnDurabilityChanged_Implementation(const float NewDurability) const
{
	OnDurabilityChanged.Broadcast(NewDurability, GetDurability());
}

void UShieldAttackComp::PlayShieldBrokenAnimation()
{
	if (!OwnerCharacter || !ShieldBrokenAnimation)
	{
		return;
	}
	if (OwnerCharacter->HasAuthority())
	{
		Multicast_PlayShieldBrokenAnimation();
	}
	else
	{
		Server_PlayShieldBrokenAnimation();
	}
}

void UShieldAttackComp::Server_PlayShieldBrokenAnimation_Implementation()
{
	Multicast_PlayShieldBrokenAnimation();
}

void UShieldAttackComp::Multicast_PlayShieldBrokenAnimation_Implementation()
{
	if (OwnerCharacter && ShieldBrokenAnimation)
	{
		OwnerCharacter->PlayAnimMontage(ShieldBrokenAnimation);
	}
}

void UShieldAttackComp::Debug()
{
	Super::Debug();
	if (Shield)
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("Current shield durability: %f"), Shield->GetDurability());
	}
	UE_LOG(AttackComponentLog, Warning, TEXT("Current shield recovery rate: %f"), GetRecoveryRate());
}
