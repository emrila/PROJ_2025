// Fill out your copyright notice in the Description page of Project Settings.


#include "WizardPlayerState.h"

#include "Net/UnrealNetwork.h"

AWizardPlayerState::AWizardPlayerState()
{
	LanPlayerName = TEXT("Player");
}

void AWizardPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWizardPlayerState, DamageTaken);
	DOREPLIFETIME(AWizardPlayerState, DamageDealt);
	DOREPLIFETIME(AWizardPlayerState, LanPlayerName);
}

void AWizardPlayerState::AddDamageTaken_Implementation(float Damage)
{
	DamageTaken += Damage;
	UE_LOG(LogTemp, Warning, TEXT("Damage Taken: %f"), DamageTaken);
}

float AWizardPlayerState::GetDamageTaken()
{
	return DamageTaken;
}

void AWizardPlayerState::AddDamageDealt_Implementation(float Damage)
{
	DamageDealt += Damage;
	UE_LOG(LogTemp, Warning, TEXT("Damage Dealt: %f"), DamageDealt);
}

float AWizardPlayerState::GetDamageDealt()
{
	return DamageDealt;
}

/*void AWizardPlayerState::OnRep_LanPlayerName()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_LanPlayerName: %s"), *LanPlayerName)
}*/
