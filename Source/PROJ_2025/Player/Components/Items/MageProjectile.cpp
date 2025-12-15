

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
	ProjectileMovementComponent->bShouldBounce = false; 

	InitialLifeSpan = LifeTime;
	
	Tags.Add(TEXT("Projectile"));
}

void AMageProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicateMovement(true);

	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMageProjectile::OnProjectileOverlap);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMageProjectile::OnProjectileHit);
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
			const float OldDamageAmount = DamageAmount;
			UE_LOG(LogTemp, Warning, TEXT("Shield Damage: %f"), Shield->GetDamageAmount());
			DamageAmount += Shield->GetDamageAmount();
			//TODO: Change visuals

			UE_LOG(LogTemp, Warning, TEXT("Damage boosted from %f: to:%f"), OldDamageAmount, DamageAmount);
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
	
	if (NewProjectileSpeed <= ProjectileSpeed)
	{
		return;
	}
	
	if (ProjectileMovementComponent)
	{
		ProjectileMovementComponent->InitialSpeed = NewProjectileSpeed;
		ProjectileMovementComponent->MaxSpeed = NewProjectileSpeed;

		if (const FVector NewVelocity = ProjectileMovementComponent->Velocity; NewVelocity.SizeSquared() > KINDA_SMALL_NUMBER)
		{
			const FVector NewVelocityDir = NewVelocity.GetSafeNormal();
			ProjectileMovementComponent->Velocity = NewVelocityDir * NewProjectileSpeed;
		}
	}
}



