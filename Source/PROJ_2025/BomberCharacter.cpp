// Furkan approves of this


#include "BomberCharacter.h"

#include "Net/UnrealNetwork.h"

void ABomberCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABomberCharacter, bIsDiving);
	DOREPLIFETIME(ABomberCharacter, Health);
}

// Sets default values
ABomberCharacter::ABomberCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABomberCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABomberCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABomberCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ABomberCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	Health -= DamageAmount;
	LaunchCharacter(GetActorForwardVector() * -1555, false, false);
	if (Health <= 0)
	{
		if (CombatManager && CombatManager->HasAuthority())
		{
			CombatManager->RegisterEnemyDeath();
			UE_LOG(LogTemp, Log, TEXT("Combat Death!"));
		}
		Server_SpawnExplosion(GetActorLocation(),GetActorRotation());
		Destroy();
	}
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ABomberCharacter::Server_SpawnExplosion_Implementation(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (!HasAuthority()) return;
	
	SetActorEnableCollision(false);
	GetWorld()->SpawnActor<AActor>(ExplosionActor, GetActorLocation(),GetActorRotation());

}


