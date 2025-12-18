

#include "MageProjectile.h"

#include "EnemyBase.h"
#include "Shield.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
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
	
	Tags.Add(TEXT("Projectile"));
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

void AMageProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);

	EnemyCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMageProjectile::OnProjectileOverlap);
	WorldStaticCollisionComponent->OnComponentHit.AddDynamic(this, &AMageProjectile::OnProjectileHit);
	
	CurrentScale = EnemyCollisionComponent->GetComponentScale();
}

void AMageProjectile::OnProjectileOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex, [[maybe_unused]] bool bFromSweep,const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}
	if (OtherActor == this)
	{
		return;
	}
	if (OtherActor->IsA(AShield::StaticClass()))
	{
		AShield* Shield = Cast<AShield>(OtherActor);
		if (Shield)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s hit shield"), *GetOwner()->GetName());
			const float OldDamageAmount = DamageAmount;
			DamageAmount += Shield->GetDamageAmount();
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
		UE_LOG(LogTemp, Warning, TEXT("%s hit %s for %f damage"), *GetOwner()->GetName(), *OtherActor->GetName(), DamageAmount);

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
	if (OtherActor && OtherActor->IsA(AMageProjectile::StaticClass()))
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
	DOREPLIFETIME(AMageProjectile, DamageAmount);
	DOREPLIFETIME(AMageProjectile, ImpactParticles);
	DOREPLIFETIME(AMageProjectile, ProjectileSpeed);
	DOREPLIFETIME(AMageProjectile, PiercingAmount);
	DOREPLIFETIME(AMageProjectile, HitEnemies);
	
	DOREPLIFETIME(AMageProjectile, CurrentScale);
	DOREPLIFETIME(AMageProjectile, ScaleFactor);
	DOREPLIFETIME(AMageProjectile, AlphaElapsed);
	DOREPLIFETIME(AMageProjectile, ScaleDuration);
}

void AMageProjectile::SetImpactParticle(UNiagaraSystem* Particles)
{
	ImpactParticles = Particles;
}

void AMageProjectile::Server_SetDamageAmount_Implementation(const float NewDamageAmount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (NewDamageAmount <= 0)
	{
		return;
	}
	
	this->DamageAmount = NewDamageAmount;
}

void AMageProjectile::Server_SetProjectileSpeed_Implementation(const float NewProjectileSpeed)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (FMath::IsNearlyEqual(NewProjectileSpeed, ProjectileSpeed, 0.01f))
	{
		return;
	}
	
	if (ProjectileMovementComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("New Projectile Speed: %f"), NewProjectileSpeed);
		UE_LOG(LogTemp, Warning, TEXT("Current Velocity: %f"), ProjectileMovementComponent->Velocity.Size());
		FVector CurrentVelocity = ProjectileMovementComponent->Velocity;
		FVector NewVelocity = CurrentVelocity * NewProjectileSpeed;
		ProjectileMovementComponent->Velocity = NewVelocity;
		UE_LOG(LogTemp, Warning, TEXT("New Velocity: %f"), ProjectileMovementComponent->Velocity.Size());
	}
}



