// Furkan approves of this


#include "WizardGameState.h"

#include "VectorUtil.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"


void AWizardGameState::BeginPlay()
{
	Super::BeginPlay();
	SetHealth(MaxHealth);
}

void AWizardGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWizardGameState, Health);
	DOREPLIFETIME(AWizardGameState, MaxHealth);
	DOREPLIFETIME(AWizardGameState, HealthPercent);
	DOREPLIFETIME(AWizardGameState, MageSelected);
	DOREPLIFETIME(AWizardGameState, TankSelected);
	DOREPLIFETIME(AWizardGameState, RogueSelected);
	DOREPLIFETIME(AWizardGameState, SelectionWidgets);
}

bool AWizardGameState::SetHealth_Validate(float HealthAmount)
{
	if (HealthAmount <= 0)
	{
		return false;
	}
	return true;
}

bool AWizardGameState::SetMaxHealth_Validate(float HealthAmount)
{
	if (HealthAmount <= 0)
	{
		return false;
	}
	return true;
}

bool AWizardGameState::RestoreHealth_Validate(float RestoreAmount)
{
	if (RestoreAmount <= 0)
	{
		return false;
	}
	return true;
}

bool AWizardGameState::DamageHealth_Validate(float DamageAmount)
{
	if (DamageAmount <= 0)
	{
		return false;
	}
	return true;
}

void AWizardGameState::DamageHealth_Implementation(float DamageAmount)
{
	Health -= DamageAmount;

	Health = UE::Geometry::VectorUtil::Clamp(Health, static_cast<float>(0) , MaxHealth);
	
	HealthPercent = Health/MaxHealth;

	
	
}

void AWizardGameState::SetHealth_Implementation(float HealthAmount)
{
	Health = UE::Geometry::VectorUtil::Clamp(HealthAmount,static_cast<float>(0),MaxHealth);

	HealthPercent = Health/MaxHealth;
}

void AWizardGameState::RestoreHealth_Implementation(float RestoreAmount)
{
	Health += RestoreAmount;

	Health = UE::Geometry::VectorUtil::Clamp(Health, static_cast<float>(0) , MaxHealth);

	HealthPercent = Health/MaxHealth;
}

void AWizardGameState::SetMaxHealth_Implementation(float HealthAmount)
{
	MaxHealth = HealthAmount;
	Health = UE::Geometry::VectorUtil::Clamp(Health, static_cast<float>(0) , MaxHealth);

	HealthPercent = Health/MaxHealth;
}
