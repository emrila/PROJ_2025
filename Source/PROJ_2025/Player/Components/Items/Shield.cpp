#include "Shield.h"

#include "EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/SpecialAttackComps/ShieldAttackComp.h"


AShield::AShield()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->SetGenerateOverlapEvents(true);

	CollisionBox->SetIsReplicated(true);
	
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(RootComponent);
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ShieldMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	ShieldMesh->SetIsReplicated(true);
}

void AShield::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!OwnerCharacter)
	{
		return;
	}
	
	FVector Forward = OwnerCharacter->GetActorForwardVector();
	FVector NewLocation =
		OwnerCharacter->GetActorLocation() +
		Forward * DistanceFromPlayer +
		FVector(0.f, 0.f, VerticalOffset);

	SetActorLocation(NewLocation);
	SetActorRotation(OwnerCharacter->GetActorRotation());

	if (bShouldCheckPlayerAlive)
	{
		if (!OwnerCharacter->IsAlive())
		{
			if (UShieldAttackComp* ShieldComp = Cast<UShieldAttackComp>(OwnerCharacter->GetSecondAttackComponent()))
			{
				ShieldComp->StartAttackCooldown();
				ShieldComp->Server_DeactivateShield();
			}
			bShouldCheckPlayerAlive = false;
		}
	}
	
}

void AShield::ActivateShield()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(DurabilityTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(DurabilityTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(DurabilityTimerHandle,
			this, &AShield::TickDurability, 1.f, true);
	}
	if (GetWorld()->GetTimerManager().IsTimerActive(RecoveryTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
	}
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(true);
		if (CollisionBox)
		{
			SetActorEnableCollision(true);	
		}
	}
}

void AShield::DeactivateShield()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(RecoveryTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(RecoveryTimerHandle,
			this, &AShield::TickRecovery, RecoveryRate, true);
	}
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(false);
		if (CollisionBox)
		{
			SetActorEnableCollision(false);	
		}
	}
}

void AShield::SetOwnerCharacter(APlayerCharacterBase* NewOwnerCharacter)
{
	if (NewOwnerCharacter)
	{
		OwnerCharacter = NewOwnerCharacter;
		SetOwner(OwnerCharacter);
	}
}

void AShield::BeginPlay()
{
	Super::BeginPlay();

	bReplicates = true;
	SetReplicateMovement(true);
	
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AShield::OnShieldOverlap);
}

void AShield::TickDurability()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (Durability <= 0.f)
	{
		if (UShieldAttackComp* ShieldComp = Cast<UShieldAttackComp>(OwnerCharacter->GetSecondAttackComponent()))
		{
			ShieldComp->StartAttackCooldown();
			ShieldComp->Server_DeactivateShield();
			return;
		}
	}

	Durability -= 10.f;
}

void AShield::TickRecovery()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (UShieldAttackComp* ShieldComp = Cast<UShieldAttackComp>(OwnerCharacter->GetSecondAttackComponent()))
	{
		if (Durability >= ShieldComp->GetDurability())
		{
			GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
			return;
		}
	}
	Durability += 10.f;
}

void AShield::OnShieldOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
	{
		//Apply damage to enemy
		//Decrease durability, but with what amount?
		//SetDamageAmount(OwnerCharacter->GetSecondAttackComponent()->GetDamageAmount());
		
		FVector KnockDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		KnockDir.Z = 0.3f;

		KnockbackForce = 1000.f;

		Enemy->LaunchCharacter(KnockDir * KnockbackForce, true, true);

		if (!IFrame)
		{
			UGameplayStatics::ApplyDamage(
			Enemy, 
			DamageAmount, 
			OwnerCharacter ? OwnerCharacter->GetController() : nullptr, 
			this, 
			UDamageType::StaticClass()
			);
			Durability -= 10.f;
			IFrame = true;
			ResetIFrame();
		}

		// TODO: Find out why enemy is continually taking damage on overlap
		// Decrease Durability
	}
	
	//Decide what happens when hit by a projectile
	//Decide what happens when hit by a ground slam attack
}

void AShield::ResetIFrame()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		[this] { IFrame = false; }, 1.f, false);
}

