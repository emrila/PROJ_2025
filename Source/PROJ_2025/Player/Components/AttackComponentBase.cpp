#include "AttackComponentBase.h"

#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"

DEFINE_LOG_CATEGORY(AttackComponentLog);

UAttackComponentBase::UAttackComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UAttackComponentBase::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsAttacking)
	{
		StartAttack();
	}
}

void UAttackComponentBase::SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction)
{
	if (OwnerInputComp && OwnerInputAction)
	{
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Started, this, &UAttackComponentBase::OnPreAttack);
		OwnerInputComp->BindAction(OwnerInputAction, ETriggerEvent::Completed, this, &UAttackComponentBase::OnStartAttack);
	}
}

float UAttackComponentBase::GetCooldownDuration()
{
	return AttackCooldown * AttackSpeedModifier;
}

float UAttackComponentBase::GetDamageAmount() const
{
	return DamageAmount * AttackDamageModifier;
}

float UAttackComponentBase::GetCurrentAnimationLength() const
{
	if (AttackAnimations.Num() == 0)
	{
		return 0.f;
	}
	
	if (AttackAnimations.IsValidIndex(CurrentAnimIndex) && AttackAnimations[CurrentAnimIndex])
	{
		return AttackAnimations[CurrentAnimIndex]->GetPlayLength();
	}
	return 0.f;
}

void UAttackComponentBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(InitialDelayTimerHandle, [this]
		{
			OwnerCharacter = Cast<APlayerCharacterBase>(GetOwner());
		}, 1.f, false);
	}
}

void UAttackComponentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAttackComponentBase, AttackSpeedModifier);
	DOREPLIFETIME(UAttackComponentBase, AttackDamageModifier);
}

void UAttackComponentBase::OnPreAttack(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Started)
	{
		return;
	}
	bIsAttacking = true;
}

void UAttackComponentBase::OnStartAttack(const FInputActionInstance& InputActionInstance)
{
	if (InputActionInstance.GetTriggerEvent() != ETriggerEvent::Completed)
	{
		return;
	}
	if (!bIsAttacking)
	{
		return;
	}
	bIsAttacking = false;
}

void UAttackComponentBase::StartAttack()
{
	if (!OwnerCharacter || !bCanAttack || !OwnerCharacter->IsAlive())
	{
		return;
	}
	
	bCanAttack = false;
	
	if (const float CurrentCooldownDuration = GetCooldownDuration(); 
		!FMath::IsNearlyEqual(CurrentCooldownDuration, 0.f, 0.01f))
	{
		OnCooldownStarted.Broadcast(CurrentCooldownDuration);
		GetWorld()->GetTimerManager().SetTimer(
		CooldownTimerHandle, this, &UAttackComponentBase::Reset, CurrentCooldownDuration, false);
	}
	else
	{
		bCanAttack = true;
	}
	
	if (bDebug)
	{
		RequestDebug();
	}
}

void UAttackComponentBase::PerformAttack()
{
	SetCurrentAnimationIndex();
	PlayAttackAnimation();
}

void UAttackComponentBase::PlayAttackAnimation()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	if (OwnerCharacter->HasAuthority())
	{
		Multicast_PlayAttackAnimation();
	}
	else
	{
		Server_PlayAttackAnimation();
	}
}

void UAttackComponentBase::Server_PlayAttackAnimation_Implementation()
{
	Multicast_PlayAttackAnimation();
}

void UAttackComponentBase::Multicast_PlayAttackAnimation_Implementation()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (GetCurrentAnimationLength() > 0.f)
	{
		if (GetCurrentAnimationLength() > GetCooldownDuration())
		{
			if (const float PlayRate = GetCurrentAnimationLength() / GetCooldownDuration(); PlayRate > 3.f)
			{
				CurrentAnimationPlayRate = 3.f;  // Higher than this will make the current animations barely visible
			}
			else
			{
				CurrentAnimationPlayRate = PlayRate;
			}
		}
		else
		{
			CurrentAnimationPlayRate = 1.f;
		}
		if (OwnerCharacter->GetMesh() && OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			OwnerCharacter->PlayAnimMontage(AttackAnimations[CurrentAnimIndex], CurrentAnimationPlayRate);
		}
	}
}

void UAttackComponentBase::Multicast_SpawnImpactParticles_Implementation(const FVector& ImpactLocation)
{
	if (!ImpactParticles || !GetWorld())
	{
		return;
	} 
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles, ImpactLocation);
}

void UAttackComponentBase::SetCurrentAnimationIndex()
{
	if (AttackAnimations.Num() == 0)
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("%s, Attack animations list is empty, no animation will be played."), *FString(__FUNCTION__));
		CurrentAnimIndex = 0;
		return;
	}
	
	if (!bIsFirstAttackAnimSet || AttackAnimations.Num() == 1)
	{
		bIsFirstAttackAnimSet = true;
		CurrentAnimIndex = 0;
		return;
	}
	
	if (AttackAnimations.Num() == (CurrentAnimIndex + 1))
	{
		CurrentAnimIndex = 0;
		return;
	}
	++CurrentAnimIndex;
}

void UAttackComponentBase::Reset()
{
	bCanAttack = true;
}

void UAttackComponentBase::RequestDebug()
{
	if (OwnerCharacter)
	{
		if (OwnerCharacter->HasAuthority())
		{
			UE_LOG(AttackComponentLog, Warning, TEXT("----- Has Authority -----"));
			Debug();
		}
		else
		{
			UE_LOG(AttackComponentLog, Warning, TEXT("----- No Authority -----"));
			Server_Debug();
		}
	}
}

void UAttackComponentBase::Debug()
{
	if (!OwnerCharacter)
	{
		return;
	}
	
	UE_LOG(AttackComponentLog, Warning, TEXT("%s, Attacked with: %s"), *OwnerCharacter->GetName(), *GetName());
	UE_LOG(AttackComponentLog, Warning, TEXT("Current damage amount: %f"), GetDamageAmount());
	UE_LOG(AttackComponentLog, Warning, TEXT("Current Cooldown time: %f"), GetCooldownDuration());
	
	if (OwnerCharacter->GetCharacterMovement())
	{
		UE_LOG(AttackComponentLog, Warning, TEXT("Current Movement speed: %f"), OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed);
	}
}

void UAttackComponentBase::Server_Debug_Implementation()
{
	Debug();
}


