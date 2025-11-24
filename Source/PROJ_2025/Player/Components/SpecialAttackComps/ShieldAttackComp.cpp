#include "ShieldAttackComp.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/Character.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/Shield.h"


UShieldAttackComp::UShieldAttackComp()
{
	PrimaryComponentTick.bCanEverTick = true;
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
	
	SpawnShield();

	GetWorld()->GetTimerManager().ClearTimer(DurabilityTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		DurabilityTimerHandle,
		[this]()
		{
			if (CurrentDurability <= 0.f)
			{
				if (CurrentShield)
				{
					CurrentShield->Destroy();
					CurrentShield = nullptr;
				}
				bIsShieldActive = false;
				GetWorld()->GetTimerManager().ClearTimer(DurabilityTimerHandle);
				GetWorld()->GetTimerManager().SetTimer(RecoveryTimerHandle, this, &UShieldAttackComp::RecoverDurability, 1.f, true);
				Super::StartAttack();
			}
			else
			{
				DecreaseDurability(10.f);
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

void UShieldAttackComp::SpawnShield()
{
	if (ShieldClass)
	{
		FVector SpawnLoc = OwnerCharacter->GetActorLocation() + OwnerCharacter->GetActorForwardVector() * 120.f;
		FRotator SpawnRot = OwnerCharacter->GetActorRotation();

		FActorSpawnParameters Params;
		Params.Owner = OwnerCharacter;

		AShield* Shield = GetWorld()->SpawnActor<AShield>(ShieldClass, SpawnLoc, SpawnRot, Params);
		if (APlayerCharacterBase* PlayerCharacterBase = Cast<APlayerCharacterBase>(OwnerCharacter))
		{
			Shield->SetOwnerCharacter(PlayerCharacterBase);
		}
			

		CurrentShield = Shield;
		bIsShieldActive = true;
	}
}

void UShieldAttackComp::DecreaseDurability(float Durability)
{
	this->CurrentDurability -= Durability;
}

void UShieldAttackComp::RecoverDurability()
{
	if (CurrentDurability >= BaseDurability)
	{
		CurrentDurability = BaseDurability;
		if (GetWorld()->GetTimerManager().IsTimerActive(RecoveryTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
		}
		return;
	}
	
	CurrentDurability += 10.f;
}

float UShieldAttackComp::GetDurability()
{
	//TODO: check if upgraded
	return CurrentDurability;
}

void UShieldAttackComp::BeginPlay()
{
	Super::BeginPlay();
	
}

