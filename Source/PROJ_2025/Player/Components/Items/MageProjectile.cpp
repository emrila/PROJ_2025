

#include "MageProjectile.h"

#include "EnemyBase.h"
#include "Shield.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"


AMageProjectile::AMageProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	WorldStaticCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("WorldStaticCollisionComp"));
	RootComponent = WorldStaticCollisionComponent;
	
	EnemyCollisionComponent = CreateDefaultSubobject<USphereComponent>("EnemyCollisionComp");
	EnemyCollisionComponent->SetupAttachment(WorldStaticCollisionComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = WorldStaticCollisionComponent;
	ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false; 

	InitialLifeSpan = LifeTime;
}

void AMageProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AlphaElapsed += DeltaTime;
	const float DashAlpha = FMath::Clamp(AlphaElapsed / ScaleDuration, 0.0f, 1.0f);
	const FVector TheNewScale = FMath::Lerp(CurrentScale, CurrentScale * ScaleFactor, DashAlpha);
	EnemyCollisionComponent->SetWorldScale3D(TheNewScale);
	
	if (DashAlpha >= 1.f)
	{
		SetActorTickEnabled(false);
	}
}

void AMageProjectile::SetDamageAmount(const float NewDamageAmount)
{
	DamageAmount = NewDamageAmount;
}

void AMageProjectile::SetProjectileSpeed(const float NewProjectileSpeed) const
{
	if (FMath::IsNearlyEqual(NewProjectileSpeed, ProjectileSpeed, 0.01f))
	{
		return;
	}
	
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->InitialSpeed = NewProjectileSpeed;
		ProjectileMovementComponent->MaxSpeed = NewProjectileSpeed;
	}
}

void AMageProjectile::BeginPlay()
{
	Super::BeginPlay();

	EnemyCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMageProjectile::OnProjectileOverlap);
	WorldStaticCollisionComponent->OnComponentHit.AddDynamic(this, &AMageProjectile::OnProjectileHit);
	
	CurrentScale = EnemyCollisionComponent->GetComponentScale();
}

void AMageProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult
	)
{
	if (!HasAuthority())
	{
		return;
	}
	
	if (OtherActor->IsA(AShield::StaticClass()))
	{
		if (AShield* Shield = Cast<AShield>(OtherActor))
		{
			DamageAmount += Shield->GetDamageAmount(); // Correct damage amount?
			SetActorTickEnabled(true);
		}
	}
	
	AActor* DamageCauser = GetOwner() ? GetOwner() : this;
	if (OtherActor && OtherActor->IsA(AEnemyBase::StaticClass()))
	{
		if (HitEnemies.Contains(OtherActor))
		{
			return;
		}
		UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, GetOwner()->GetInstigatorController(), DamageCauser, nullptr);
		HitEnemies.Add(OtherActor);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles, SweepResult.ImpactPoint);
		
		if (HitEnemies.Num() >= PiercingAmount)
		{
			Destroy();
		}
	}
}

void AMageProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}
	if (OtherActor && OtherActor->IsA(StaticClass()))
	{
		return;
	}
	
	if (OtherComp)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles,Hit.ImpactPoint);
		Destroy();
	}
}

void AMageProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}



