#include "ChronoRiftZone.h"

#include "EnemyBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "ChronoRiftDamageType.h"

AChronoRiftZone::AChronoRiftZone()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComponent;
	
	SphereComponent->SetSphereRadius(Radius);
	
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetGenerateOverlapEvents(true);

	SphereComponent->SetIsReplicated(true);
}

void AChronoRiftZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChronoRiftZone::SetOwnerCharacter(ACharacter* NewOwnerCharacter)
{
	OwnerCharacter = NewOwnerCharacter;
}

void AChronoRiftZone::SetInitialValues(const float NewRadius, const float NewLifetime, const float NewDamageAmount)
{
	if (OwnerCharacter && OwnerCharacter->GetController())
	{
		SetOwner(OwnerCharacter->GetController()); //Must be done for network replication to work
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s, OwnerCharacter is not set"), *FString(__FUNCTION__));
	}
	Radius = NewRadius;
	Lifetime = NewLifetime;
	DamageAmount = NewDamageAmount;
}

void AChronoRiftZone::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ChronoRiftEffect);
}


void AChronoRiftZone::BeginPlay()
{
	Super::BeginPlay();
	
	EnemiesToGiveDamage.Empty();
	EnemiesSLowedDown.Empty();
	
	GetWorld()->GetTimerManager().SetTimer(TickDamageTimerHandle, this, &AChronoRiftZone::TickDamage, 1.f, true);
	
	FTimerHandle InitialDelayTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		InitialDelayTimerHandle,
		[this]()
		{
			MakeInitialSphereSweep();
			Server_SpawnEffect();
			SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AChronoRiftZone::OnOverlapBegin);
			SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AChronoRiftZone::OnOverlapEnd);
			DestroySelf();
		},
		0.5f,
		false
	);
}

void AChronoRiftZone::MakeInitialSphereSweep()
{
	SphereComponent->SetSphereRadius(Radius);
	/*DrawDebugSphere(GetWorld(), SphereComponent->GetComponentLocation(),
		SphereComponent->GetScaledSphereRadius(), 32, FColor::Purple, false, Lifetime);*/
	TArray<AActor*> OverlappedActors;
	SphereComponent->GetOverlappingActors(OverlappedActors);

	for (AActor* TheActor : OverlappedActors)
	{
		if (TheActor->IsA(AEnemyBase::StaticClass()))
		{
			EnemiesToGiveDamage.Add(TheActor);
			Server_SlowEnemy(TheActor);
		}
	}
}

void AChronoRiftZone::DestroySelf()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this] ()
		{
			Destroy();
		},
		Lifetime + 0.2f, false);
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
	}
}

void AChronoRiftZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	EnemiesToGiveDamage.Remove(OtherActor);
}

void AChronoRiftZone::TickDamage()
{
	for (AActor* Enemy : EnemiesToGiveDamage)
	{
		if (IsValid(Enemy))
		{
			UGameplayStatics::ApplyDamage(
			Enemy, 
			DamageAmount, 
			OwnerCharacter ? OwnerCharacter->GetController() : nullptr, 
			this, 
			UChronoRiftDamageType::StaticClass()
			);
		}
	}
}

void AChronoRiftZone::Server_SpawnEffect_Implementation()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
	{
		return;
	}
	Multicast_SpawnEffect();
}


void AChronoRiftZone::Multicast_SpawnEffect_Implementation()
{
	if (!ChronoRiftEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, ChronoRiftEffect is NULL!"), *FString(__FUNCTION__));
		return;
	}
	
	if (!SphereComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("%s, SphereComponent is NULL!"), *FString(__FUNCTION__));
		return;
	}
	//Set Scale and Duration here before spawning
	const FVector SpawnLocation = GetActorLocation();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ChronoRiftEffect, SpawnLocation);
}
