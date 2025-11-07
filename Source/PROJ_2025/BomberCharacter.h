// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "CombatManager.h"
#include "GameFramework/Character.h"
#include "BomberCharacter.generated.h"


UCLASS()
class PROJ_2025_API ABomberCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	// Sets default values for this character's properties
	ABomberCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsDiving = false;

	UFUNCTION(Server, Reliable)
	void Server_SpawnExplosion(FVector SpawnLocation, FRotator SpawnRotation);

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ExplosionActor;
	

	UPROPERTY(Replicated, BlueprintReadOnly)
	float Health = 10.f;

	UPROPERTY()
	ACombatManager* CombatManager;

};
