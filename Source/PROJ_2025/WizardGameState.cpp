// Furkan approves of this


#include "WizardGameState.h"

#include "RoomLoader.h"
#include "VectorUtil.h"
#include "WizardGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"


void AWizardGameState::BeginPlay()
{
	Super::BeginPlay();
	SetHealth(MaxHealth);
}

void AWizardGameState::PlayerEnteredStartDungeon_Implementation(bool Entered)
{
	if (Entered)
	{
		PlayersInStartDungeon++;
	}else
	{
		PlayersInStartDungeon--;
	}
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
	DOREPLIFETIME(AWizardGameState, CurrentPlayerCount);
	DOREPLIFETIME(AWizardGameState, PlayersInStartDungeon);
	
}


void AWizardGameState::DamageHealth_Implementation(float DamageAmount)
{
	Health -= DamageAmount;

	Health = UE::Geometry::VectorUtil::Clamp(Health, static_cast<float>(0) , MaxHealth);
	
	HealthPercent = Health/MaxHealth;
	
	UE_LOG(LogTemp, Error, TEXT("Damage Taken"));
	

	if (HealthPercent <= 0 && Cast<UWizardGameInstance>(GetGameInstance())->RoomLoader->CurrentRoom.RoomData->RoomType != ERoomType::Parkour)
	{
		for (APlayerState* Player : PlayerArray)
		{
			if (Player->GetPawn())
			{
				Cast<APlayerCharacterBase>(Player->GetPawn())->StartSuddenDeath();
			}
		}
	}
	else if (HealthPercent <= 0){
		for (APlayerState* Player : PlayerArray)
		{
			if (Player->GetPawn())
			{
				Cast<APlayerCharacterBase>(Player->GetPawn())->SetIsAlive(false);
			}
		}
	}
	OnRep_Health();
	ForceNetUpdate();
}

void AWizardGameState::SetHealth_Implementation(float HealthAmount)
{
	Health = UE::Geometry::VectorUtil::Clamp(HealthAmount,static_cast<float>(0),MaxHealth);

	HealthPercent = Health/MaxHealth;

	if (HealthPercent > 0)
	{
		for (APlayerState* Player : PlayerArray)
		{
			if (Player->GetPawn())
			{
				if (APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(Player->GetPawn()))
				{
					PlayerCharacter->EndSuddenDeath();
					PlayerCharacter->SetIsAlive(true);
				}
			}
		}
	}
	if (HealthPercent <= 0)
	{
		for (APlayerState* Player : PlayerArray)
		{
			if (Player->GetPawn())
			{
				Cast<APlayerCharacterBase>(Player->GetPawn())->StartSuddenDeath();
			}
		}
	}
	OnRep_Health();
	ForceNetUpdate();
}

void AWizardGameState::RestoreHealth_Implementation(float RestoreAmount)
{
	Health += RestoreAmount;

	Health = UE::Geometry::VectorUtil::Clamp(Health, static_cast<float>(0) , MaxHealth);

	HealthPercent = Health/MaxHealth;

	if (HealthPercent > 0)
	{
		for (APlayerState* Player : PlayerArray)
		{
			if (Player->GetPawn())
			{
				if (APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>(Player->GetPawn()))
				{
					PlayerCharacter->EndSuddenDeath();
					PlayerCharacter->SetIsAlive(true);
				}
			}
		}
	}
	OnRep_Health();
	ForceNetUpdate();
}

void AWizardGameState::SetMaxHealth_Implementation(float HealthAmount)
{
	MaxHealth = HealthAmount;
	Health = UE::Geometry::VectorUtil::Clamp(Health, static_cast<float>(0) , MaxHealth);

	HealthPercent = Health/MaxHealth;

	OnRep_Health();
	ForceNetUpdate();
}
