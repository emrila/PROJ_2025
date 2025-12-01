#include "Shield.h"

#include "EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Components/SpecialAttackComps/ShieldAttackComp.h"


AShield::AShield()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	
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
}

void AShield::RequestActivateShield()
{
	Server_ActivateShield();
}

void AShield::Server_ActivateShield_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	Multicast_ActivateShield();

	// Start durability timer on server
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DurabilityTimerHandle);

		// Clear any existing timer to avoid having multiples running
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);

		GetWorld()->GetTimerManager().SetTimer(DurabilityTimerHandle, this, &AShield::TickDurability, 1.f, true);
	}
}

void AShield::Multicast_ActivateShield_Implementation()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (ShieldMesh && CollisionBox)
	{
		ShieldMesh->SetVisibility(true);
		SetActorEnableCollision(true);
	}
}

void AShield::RequestDeactivateShield()
{
	Server_DeactivateShield();
}

void AShield::Server_DeactivateShield_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	Multicast_DeactivateShield();
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DurabilityTimerHandle);

		// Clear any existing timer to avoid having multiples running
		GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);

		GetWorld()->GetTimerManager().SetTimer(RecoveryTimerHandle, this, &AShield::TickRecovery, 1.f, true);
	}
}

void AShield::Multicast_DeactivateShield_Implementation()
{
	if (!OwnerCharacter)
	{
		return;
	}
	if (ShieldMesh && CollisionBox)
	{
		ShieldMesh->SetVisibility(false);
		SetActorEnableCollision(false);
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
	
	if (HasAuthority() && CollisionBox)
	{
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AShield::OnShieldOverlap);
	}

	if (CollisionBox && ShieldMesh)
	{
		ShieldMesh->SetVisibility(false);
		SetActorEnableCollision(false);
	}
}

void AShield::TickDurability()
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (!OwnerCharacter)
	{
		return;
	}
	if (Durability <= 0.f)
	{
		if (UShieldAttackComp* ShieldComp = Cast<UShieldAttackComp>(OwnerCharacter->GetSecondAttackComponent()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Durability is:%f , Deactivating the shield."), Durability);
			ShieldComp->StartAttackCooldown();
			ShieldComp->DeactivateShield();
			return;
		}
	}
	const float OldDurability = Durability;
	Durability -= 10.f;
	UE_LOG(LogTemp, Warning, TEXT("Durability reduced from:%f, to:%f"), OldDurability, Durability);
}

void AShield::TickRecovery()
{
	if (!HasAuthority())
	{
		return;
	}
	if (!OwnerCharacter)
	{
		return;
	}
	if (UShieldAttackComp* ShieldComp = Cast<UShieldAttackComp>(OwnerCharacter->GetSecondAttackComponent()))
	{
		if (Durability >= ShieldComp->GetDurability())
		{
			Durability = ShieldComp->GetDurability();
			GetWorld()->GetTimerManager().ClearTimer(RecoveryTimerHandle);
			return;
		}
	}
	const float OldDurability = Durability;
	Durability += 10.f;
	UE_LOG(LogTemp, Warning, TEXT("Durability increased from:%f, to:%f"), OldDurability, Durability);
}

void AShield::OnShieldOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
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
			OwnerCharacter ? OwnerCharacter->GetController() : nullptr, 
			this, 
			UDamageType::StaticClass()
			);
			Durability -= 10.f;
			bShouldGiveDamage = false;
			ResetShouldGiveDamage();
		}
	}
}

void AShield::ResetShouldGiveDamage()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		[this] { bShouldGiveDamage = true; }, 1.f, false);
}

void AShield::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShield, Durability)
}

