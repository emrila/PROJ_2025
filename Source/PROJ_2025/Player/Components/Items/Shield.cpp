#include "Shield.h"

#include "EnemyBase.h"
#include "TrapDamageType.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/SpecialAttackComps/ShieldAttackComp.h"

DEFINE_LOG_CATEGORY(ShieldLog);


AShield::AShield()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;
	
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	RootComponent = ShieldMesh;
	
	ShieldMesh->OnComponentHit.AddDynamic(this, &AShield::OnShieldHit);
}

void AShield::ActivateShield()
{
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(true);
		SetActorEnableCollision(true);
	}
	
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
	}
}

void AShield::DeactivateShield()
{
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(false);
		SetActorEnableCollision(false);
	}
	
	if (GetWorld())
	{
		if (!GetWorld()->GetTimerManager().IsTimerActive(RecoveryTimerHandle))
		{
			GetWorld()->GetTimerManager().SetTimer(
				RecoveryTimerHandle, this, &AShield::TickRecovery, RecoveryRate, true);
		}
	}
}

void AShield::SetOwnerProperties(APlayerCharacterBase* NewOwnerCharacter,
	UShieldAttackComp* NewOwnerAttackComponent, const float NewDurability)
{
	if (NewOwnerCharacter && NewOwnerAttackComponent)
	{
		OwnerCharacter = NewOwnerCharacter;
		OwnerAttackComponent = NewOwnerAttackComponent;
		Durability = NewDurability;
	}
}

void AShield::SetValuesPreActivation(const float NewDamageAmount, const float NewRecoveryRate)
{
	DamageAmount = NewDamageAmount;
	RecoveryRate = NewRecoveryRate;
}

void AShield::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);
	
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(false);
		SetActorEnableCollision(false);
	}
}

void AShield::TickRecovery()
{
	if (!OwnerAttackComponent)
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
		return;
	}
	if (Durability >= OwnerAttackComponent->GetDurability())
	{
		Durability = OwnerAttackComponent->GetDurability();
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
		return;
	}
	
	ChangeDurability(true, 10.f);
}

void AShield::OnShieldHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || !OwnerCharacter)
	{
		return;
	}
	
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
	{
		if (bShouldGiveDamage)
		{
			UGameplayStatics::ApplyDamage(
			Enemy, 
			DamageAmount, 
			OwnerCharacter->GetController() ? OwnerCharacter->GetController() : nullptr, 
			OwnerCharacter,
			UDamageType::StaticClass()
			);
			ChangeDurability(false, 10.f);
			ResetShouldGiveDamage();
		}
	}
}

float AShield::TakeDamage(float NewDamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()  || DamageEvent.DamageTypeClass == UTrapDamageType::StaticClass())
	{
		return 0.f;
	}
	
	ChangeDurability(false, 10.f);
	
	return 10.f;
}

void AShield::ChangeDurability(bool bIncrease, const float AmountToChange)
{
	if (!OwnerAttackComponent)
	{
		return;
	}
	
	if (bIncrease)
	{
		Durability += AmountToChange;
	}
	else
	{
		Durability -= AmountToChange;
		if (Durability <= 0.f)
		{
			Durability = 0.f;
			DeactivateShield();
			OwnerAttackComponent->HandleCooldown();
		}
	}
	OwnerAttackComponent->HandleOnDurabilityChanged(Durability);
}

void AShield::ResetShouldGiveDamage()
{
	bShouldGiveDamage = false;
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		[this] { bShouldGiveDamage = true; }, 1.f, false);
}

