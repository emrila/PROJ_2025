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
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &UShieldAttackComp::Test);
	}
}

void UShieldAttackComp::Test()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	if (bIsTest)
	{
		if (CurrentShield)
		{
			CurrentShield->Destroy();
			CurrentShield = nullptr;
			bIsTest = false;
		}
	}
	else
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
			bIsTest = true;
		}
	}
}

void UShieldAttackComp::BeginPlay()
{
	Super::BeginPlay();
	
}

