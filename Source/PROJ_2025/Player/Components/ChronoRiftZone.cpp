#include "ChronoRiftZone.h"

#include "EnemyBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"

AChronoRiftZone::AChronoRiftZone()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComponent;
	
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetHiddenInGame(true);
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->SetSphereRadius(Radius);
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AChronoRiftZone::OnOverlapBegin);
	
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(true);
}

AChronoRiftZone::AChronoRiftZone(float NewRadius, float NewLifetime, float NewDamageAmount)
{
	AChronoRiftZone();
	Radius = NewRadius;
	Lifetime = NewLifetime;
	DamageAmount = NewDamageAmount;
}

void AChronoRiftZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChronoRiftZone::SetOwnerCharacter(APlayerCharacterBase* NewOwnerCharacter)
{
	OwnerCharacter = NewOwnerCharacter;
}


void AChronoRiftZone::BeginPlay()
{
	Super::BeginPlay();
	
	EnemiesToGiveDamage.Empty();
	
	GetWorld()->GetTimerManager().SetTimer(TickDamageTimerHandle, this, &AChronoRiftZone::Server_TickDamage, 1.f, true);
	
	Multicast_SpawnEffect();
}

void AChronoRiftZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GetWorld()->GetTimerManager().ClearTimer(TickDamageTimerHandle);
	EnemiesToGiveDamage.Empty();
	
	Server_ResetEnemiesPreEnd();
}

void AChronoRiftZone::Server_SlowEnemy_Implementation(AActor* Enemy)
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	if (!Enemy)
	{
		return;
	}
	
	Multicast_SlowEnemy(Enemy);
}

void AChronoRiftZone::Multicast_SlowEnemy_Implementation(AActor* Enemy)
{
	if (!IsValid(Enemy))
	{
		return;
	}
	const float AppliedDilation = FMath::Clamp(EnemyTimeDilationFactor, 0.01f, 1.0f);
		
	Enemy->CustomTimeDilation = AppliedDilation;
		
	if (APawn* Pawn = Cast<APawn>(Enemy))
	{
		if (AController* C = Pawn->GetController())
		{
			C->CustomTimeDilation = AppliedDilation;
			C->ForceNetUpdate();
		}
	}
	Enemy->ForceNetUpdate();
	EnemiesSLowedDown.Add(Enemy);
}

void AChronoRiftZone::Server_ResetEnemy_Implementation(AActor* Enemy)
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	if (!Enemy)
	{
		return;
	}
	
	Multicast_ResetEnemy(Enemy);
}

void AChronoRiftZone::Multicast_ResetEnemy_Implementation(AActor* Enemy)
{
	if (!IsValid(Enemy))
	{
		return;
	}
	
	FTimerHandle TimerHandle;
	
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this, Enemy]()
		{
			if (Enemy->CustomTimeDilation == 1.f)
			{
				return;
			}
			Enemy->CustomTimeDilation = 1.f;
			if (APawn* Pawn = Cast<APawn>(Enemy))
			{
				if (AController* C = Pawn->GetController())
				{
					C->CustomTimeDilation = 1.f;
					C->ForceNetUpdate();
				}
			}
			Enemy->ForceNetUpdate();
		},
		Lifetime,
		false
	);
}

void AChronoRiftZone::Server_ResetEnemiesPreEnd_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	
	Multicast_ResetEnemiesPreEnd();
}

void AChronoRiftZone::Multicast_ResetEnemiesPreEnd_Implementation()
{
	for (AActor* Enemy : EnemiesSLowedDown)
	{
		if (IsValid(Enemy))
		{
			if (Enemy->CustomTimeDilation == 1.f)
			{
				continue;
			}
			Enemy->CustomTimeDilation = 1.f;
			if (APawn* Pawn = Cast<APawn>(Enemy))
			{
				if (AController* C = Pawn->GetController())
				{
					C->CustomTimeDilation = 1.f;
					C->ForceNetUpdate();
				}
			}
			Enemy->ForceNetUpdate();
		}
	}
}

void AChronoRiftZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(AEnemyBase::StaticClass()))
	{
		EnemiesToGiveDamage.Add(OtherActor);
		Server_SlowEnemy(OtherActor);
		Server_ResetEnemy(OtherActor);
	}
}

void AChronoRiftZone::Server_TickDamage_Implementation()
{
	if (EnemiesToGiveDamage.Num() == 0)
	{
		return;
	}
	
	for (AActor* Enemy : EnemiesToGiveDamage)
	{
		if (IsValid(Enemy))
		{
			UGameplayStatics::ApplyDamage(
			Enemy, 
			DamageAmount, 
			OwnerCharacter ? OwnerCharacter->GetController() : nullptr, 
			this, 
			UDamageType::StaticClass()
			);
		}
	}
}

void AChronoRiftZone::Multicast_SpawnEffect_Implementation()
{
	if (!ChronoRiftEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, ChronoRiftEffect is NULL!"), *FString(__FUNCTION__));
		return;
	}
	
	//Set Scale and Duration here before spawning
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ChronoRiftEffect, GetActorLocation());
}
