#include "ChronoRiftZone.h"

#include "EnemyBase.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "ChronoRiftDamageType.h"
#include "TrapBase.h"
#include "NiagaraComponent.h"

AChronoRiftZone::AChronoRiftZone()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComponent;
	
	SphereComponent->SetSphereRadius(Radius);
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AChronoRiftZone::OnOverlapBegin);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AChronoRiftZone::OnOverlapEnd);
	
	bReplicates = true;
}

void AChronoRiftZone::SetOwnerCharacter(APlayerCharacterBase* NewOwnerCharacter)
{
	OwnerCharacter = NewOwnerCharacter;
}

void AChronoRiftZone::SetInitialValues(const float NewRadius, const float NewLifetime, const float NewDamageAmount)
{
	Radius = NewRadius;
	Lifetime = NewLifetime;
	DamageAmount = NewDamageAmount;
}

void AChronoRiftZone::BeginPlay()
{
	Super::BeginPlay();
	RequestSpawnEffect();
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
		TickDamageTimerHandle,
		this, 
		&AChronoRiftZone::TickDamage,
		1.f,
		true
		);
		MakeInitialSphereSweep();
		HandleDestroySelf();
	}
}

void AChronoRiftZone::MakeInitialSphereSweep()
{
	if (!SphereComponent)
	{
		return;
	}
	SphereComponent->SetSphereRadius(Radius);
	TArray<AActor*> OverlappedActors;
	SphereComponent->GetOverlappingActors(OverlappedActors);

	for (AActor* Actor : OverlappedActors)
	{
		if (Actor->IsA(AEnemyBase::StaticClass()))
		{
			HandleOverlapBegin(Actor);
		} 
	}
}

void AChronoRiftZone::TickDamage()
{
	if (!OwnerCharacter || !OwnerCharacter->GetController())
	{
		return;
	}
	
	for (AActor* Enemy : OverlappedEnemies)
	{
		if (Enemy)
		{
			UGameplayStatics::ApplyDamage(
			Enemy, 
			DamageAmount, 
			OwnerCharacter->GetController(), 
			OwnerCharacter, 
			UChronoRiftDamageType::StaticClass()
			);
		}
	}
}

void AChronoRiftZone::HandleDestroySelf()
{
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this] ()
		{
			GetWorld()->GetTimerManager().ClearTimer(TickDamageTimerHandle);
			for (AActor* Actor: OverlappedEnemies)
			{
				if (Actor)
				{
					Actor->CustomTimeDilation = 1.f;
				}
			}
			Destroy();
		},
		Lifetime + 0.2f, false);
}

void AChronoRiftZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	if (OtherActor)
	{
		if (OtherActor->IsA(AEnemyBase::StaticClass()) || OtherActor->IsA(ATrapBase::StaticClass()))
		{
			HandleOverlapBegin(OtherActor);
		}
	}
}

void AChronoRiftZone::HandleOverlapBegin(AActor* OtherActor)
{
	if (OtherActor)
	{
		OverlappedEnemies.Add(OtherActor);
		OtherActor->CustomTimeDilation = FMath::Clamp(EnemyTimeDilationFactor, 0.01f, 1.0f);
	}
}

void AChronoRiftZone::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}
	HandleOverlapEnd(OtherActor);
}

void AChronoRiftZone::HandleOverlapEnd(AActor* OtherActor)
{
	if (OtherActor && OverlappedEnemies.Contains(OtherActor))
 	{
		OverlappedEnemies.Remove(OtherActor);
		OtherActor->CustomTimeDilation = 1.f;
 	}
}

void AChronoRiftZone::RequestSpawnEffect()
{
	if (HasAuthority())
	{
		Multicast_SpawnEffect(Lifetime, InitialRadius, Radius);
	}
	else
	{
		Server_SpawnEffect(Lifetime, InitialRadius, Radius);
	}
}

void AChronoRiftZone::SpawnEffect(const float NewChronoDuration, const float BaseRadius, const float CurrentRadius) const
{
	if (!ChronoRiftEffect)
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("ChronoRiftEffect is not null"));
	UNiagaraComponent* CurrentNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ChronoRiftEffect,
		GetActorLocation(),
		FRotator::ZeroRotator,
		FVector(1.f),
		true,
		false
		);
	
	if (CurrentNiagaraComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned ChronoRiftEffect"));
		const float NiagaraRadius = CurrentRadius / BaseRadius;
		CurrentNiagaraComponent->SetVariableFloat(TEXT("ChronoRadius"), NiagaraRadius);
		CurrentNiagaraComponent->SetVariableFloat(TEXT("ChronoDuration"), NewChronoDuration);
		CurrentNiagaraComponent->ActivateSystem();
	}
	
}

void AChronoRiftZone::Server_SpawnEffect_Implementation(const float NewLifeTime, const float BaseRadius, const float CurrentRadius)
{
	Multicast_SpawnEffect(NewLifeTime, BaseRadius, CurrentRadius);
}


void AChronoRiftZone::Multicast_SpawnEffect_Implementation(const float NewLifeTime, const float BaseRadius, const float CurrentRadius)
{
	SpawnEffect(NewLifeTime, BaseRadius, CurrentRadius);
}
