

#include "MageProjectile.h"

#include "EnemyBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Characters/PlayerCharacterBase.h"


AMageProjectile::AMageProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	this->SetActorScale3D(FVector(5, 5,5));

	CollisionComponent->SetCollisionProfileName(FName("Projectile"));
	RootComponent = CollisionComponent;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ProjectileMesh->SetupAttachment(CollisionComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->UpdatedComponent = CollisionComponent;
	ProjectileMovementComponent->InitialSpeed = ProjectileSpeed;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;  //TODO: Should it bounce?

	InitialLifeSpan = LifeTime;
	
	Tags.Add(TEXT("Projectile"));
}

void AMageProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMageProjectile::OnProjectileOverlap);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMageProjectile::OnProjectileHit);
}

void AMageProjectile::OnProjectileOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex, [[maybe_unused]] bool bFromSweep,const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(AEnemyBase::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s hit %s for %f damage"), *GetOwner()->GetName(), *OtherActor->GetName(), DamageAmount);

		UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, GetOwner()->GetInstigatorController(), this, nullptr);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles, SweepResult.ImpactPoint);

		Destroy();	
	}
}

void AMageProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherComp->IsA(StaticClass()))
	{
		return;
	}
	
	if (OtherComp)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles,Hit.ImpactPoint);
		Destroy();
	}
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



