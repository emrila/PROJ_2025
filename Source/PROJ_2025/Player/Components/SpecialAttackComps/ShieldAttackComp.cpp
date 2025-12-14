#include "ShieldAttackComp.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Items/Shield.h"


UShieldAttackComp::UShieldAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	DamageAmount = 5.f;
	AttackCooldown = 5.f;
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

	if (!OwnerCharacter->IsAlive())
	{
		return;
	}
	
	if (!bIsShieldActive)
	{
		ActivateShield();
		return;
	}
	DeactivateShield();
}

void UShieldAttackComp::SpawnShield()
{
	if (!OwnerCharacter || !ShieldClass)
	{
		return;
	}

	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}

	FVector SpawnLoc = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 120.f;
	FRotator SpawnRot = OwnerCharacter->GetActorRotation();

	FActorSpawnParameters Params;
	Params.Owner = OwnerCharacter;

	if (!OwnerCharacter->GetInstigator())
	{
		UE_LOG(LogTemp, Warning, TEXT("OwnerCharacter's Instigator is NULL in %s"), *FString(__FUNCTION__));
		return;
	}
	
	Params.Instigator = OwnerCharacter->GetInstigator();
	
	AShield* Shield = GetWorld()->SpawnActor<AShield>(ShieldClass, SpawnLoc, SpawnRot, Params);
	if (!Shield)
	{
		UE_LOG(LogTemp, Warning, TEXT("Shield spawn failed in %s"), *FString(__FUNCTION__));
		return;
	}
	CurrentShield = Shield;
	CurrentShield->SetOwnerCharacter(Cast<APlayerCharacterBase>(OwnerCharacter));

	CurrentShield->SetDurability(GetDurability());
	CurrentShield->SetDamageAmount(GetDamageAmount());
	CurrentShield->SetRecoveryRate(GetRecoveryRate());
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

void UShieldAttackComp::ActivateShield()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (!CurrentShield)
	{
		TArray<AActor*> ShieldActors;
		OwnerCharacter->GetAllChildActors(ShieldActors);

		OwnerCharacter->OnPlayerDied.AddDynamic(this, &UShieldAttackComp::OnPlayerDied);

		for (AActor* Actor : ShieldActors)
		{
			if (AShield* Shield = Cast<AShield>(Actor))
			{
				CurrentShield = Shield;
				break;
			}
		}
	}
	
	if (!CurrentShield)
	{
		return;
	}

	bIsShieldActive = true;
	//OwnerCharacter->SetShouldUseSprintInput(false);

	// Update shield properties before activation in case of modifiers change, Durability is not updated here because Shield handles it internally
	CurrentShield->SetDamageAmount(GetDamageAmount());
	CurrentShield->SetRecoveryRate(GetRecoveryRate());
	CurrentMoveSpeed = OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;
	HandleOwnerMovement(CurrentMoveSpeed * CurrentShield->GetPlayerMovementSpeedMultiplier());
	CurrentShield->RequestActivateShield();
}

void UShieldAttackComp::DeactivateShield()
{
	if (!OwnerCharacter || !CurrentShield)
	{
		return;
	}
	bIsShieldActive = false;
	//OwnerCharacter->SetShouldUseSprintInput(true);
	
	HandleOwnerMovement(CurrentMoveSpeed);
	CurrentShield->RequestDeactivateShield();
}

void UShieldAttackComp::StartAttackCooldown()
{
	if (!OwnerCharacter || !bCanAttack)
	{
		return;
	}

	Multicast_StartAttackCooldown();
}

void UShieldAttackComp::Multicast_StartAttackCooldown_Implementation()
{
	bCanAttack = false;

	// Reset shield durability to max on cooldown start
	if (CurrentShield)
	{
		CurrentShield->SetDurability(GetDurability());
	}
	HandleOwnerMovement(CurrentMoveSpeed);
	
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UShieldAttackComp::ResetAttackCooldown, GetAttackCooldown(), false);
}

void UShieldAttackComp::BeginPlay()
{
	Super::BeginPlay();

	if (OwnerCharacter)
	{
		TArray<AActor*> ShieldActors;
		OwnerCharacter->GetAllChildActors(ShieldActors);

		OwnerCharacter->OnPlayerDied.AddDynamic(this, &UShieldAttackComp::OnPlayerDied);

		for (AActor* Actor : ShieldActors)
		{
			if (AShield* Shield = Cast<AShield>(Actor))
			{
				CurrentShield = Shield;
				break;
			}
		}

		if (!CurrentShield)
		{
			UE_LOG(LogTemp, Error, TEXT("No shield found in %s"), *FString(__FUNCTION__));
			return;
		}
		
		CurrentShield->SetOwnerCharacter(Cast<APlayerCharacterBase>(OwnerCharacter));

		CurrentShield->SetDurability(GetDurability());
		CurrentShield->SetDamageAmount(GetDamageAmount());
		CurrentShield->SetRecoveryRate(GetRecoveryRate());
	}
}

void UShieldAttackComp::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UShieldAttackComp, CurrentShield);
}

void UShieldAttackComp::OnPlayerDied(bool bNewIsAlive)
{
	if (!bNewIsAlive)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, Player Died, Deactivating Shield"), *FString(__FUNCTION__));
		DeactivateShield();
		ResetAttackCooldown();
	}
}

void UShieldAttackComp::ResetAttackCooldown()
{
	if (!CurrentShield)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, CurrentShield is NULL"), *FString(__FUNCTION__));
		return;
	}
	Super::ResetAttackCooldown();
	DeactivateShield();
}

void UShieldAttackComp::HandleOwnerMovement(const float NewMoveSpeed)
{
	if (!CurrentShield || !OwnerCharacter)
	{
		return;
	}
	
	if (OwnerCharacter->HasAuthority())
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = NewMoveSpeed;
	}
	else
	{
		Server_HandleOwnerMovement(NewMoveSpeed);
	}
}

void UShieldAttackComp::Server_HandleOwnerMovement_Implementation(const float NewMoveSpeed)
{
	if (!CurrentShield || !OwnerCharacter)
	{
		return;
	}
	Multicast_HandleOwnerMovement(NewMoveSpeed);
}

void UShieldAttackComp::Multicast_HandleOwnerMovement_Implementation(const float NewMoveSpeed)
{
	if (!CurrentShield || !OwnerCharacter)
	{
		return;
	}
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = NewMoveSpeed;
}


