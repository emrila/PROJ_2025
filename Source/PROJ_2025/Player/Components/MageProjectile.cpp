

#include "MageProjectile.h"

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

void AMageProjectile::OnProjectileOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (APlayerCharacterBase* Player = Cast<APlayerCharacterBase>(OtherActor))
	{
		return;
	}

	if (OtherActor)
	{
		UGameplayStatics::ApplyDamage(OtherActor, DamageAmount, GetOwner()->GetInstigatorController(), this, nullptr);
		Destroy();
		return;
	}
}

void AMageProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherComp)
	{
		Destroy();
	}
}



