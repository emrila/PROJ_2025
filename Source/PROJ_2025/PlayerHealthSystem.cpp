// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHealthSystem.h"

#include "VectorUtil.h"

void UPlayerHealthSystem::DamageHealth(const float DamageAmount)
{
	Health -= DamageAmount;

	/*
	 * if(Health <= 0)
	 *		GameOver
	 */
}

void UPlayerHealthSystem::RestoreHealth(const float RestoreAmount)
{
	Health += RestoreAmount;
}

float UPlayerHealthSystem::GetHealth() const
{
	return Health;
}

float UPlayerHealthSystem::GetHealthPercent() const
{
	return Health/MaxHealth;
}

void UPlayerHealthSystem::SetMaxHealth(float HealthAmount)
{
	MaxHealth = HealthAmount;
}

void UPlayerHealthSystem::SetHealth(float HealthAmount)
{
	Health = UE::Geometry::VectorUtil::Clamp(HealthAmount,static_cast<float>(0),MaxHealth);
}
