

#include "Shield.h"

#include "AttackComponentBase.h"
#include "EnemyBase.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"


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
	
	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(RootComponent);
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ShieldMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
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
}

void AShield::ActivateShield()
{
	if (!bCanBeActivated)
	{
		return;
	}
	//Update durability if upgraded
	//Set a timer for durability reduction
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
	//Set a timer for durability recovery
	if (ShieldMesh)
	{
		ShieldMesh->SetVisibility(false);
		if (CollisionBox)
		{
			SetActorEnableCollision(false);	
		}
	}
}

void AShield::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AShield::OnShieldOverlap);
}

void AShield::OnShieldOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == OwnerCharacter)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Overlap with: %s"), *OtherActor->GetName());
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor))
	{
		//Apply damage to enemy
		//Decrease durability, but with what amount?
		//SetDamageAmount(OwnerCharacter->GetSecondAttackComponent()->GetDamageAmount());
		UGameplayStatics::ApplyDamage(
			Enemy, 
			DamageAmount, 
			OwnerCharacter ? OwnerCharacter->GetController() : nullptr, 
			this, 
			UDamageType::StaticClass()
			);
		
		FVector KnockDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		KnockDir.Z = 0.3f;

		const float KnockbackStrength = 1000.f;

		Enemy->LaunchCharacter(KnockDir * KnockbackStrength, true, true);
	}
	
	//Decide what happens when hit by a projectile
	//Decide what happens when hit by a ground slam attack
}

