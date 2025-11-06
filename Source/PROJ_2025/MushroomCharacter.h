// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "CombatManager.h"
#include "GameFramework/Character.h"
#include "MushroomCharacter.generated.h"

UCLASS()
class PROJ_2025_API AMushroomCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMushroomCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Multicast_Jump(float Angle, FRotator RotationToPlayer, float JumpStrength, float ForwardStrength);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool AttackIsOnCooldown = false;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsAttacking;

	UPROPERTY(Replicated, BlueprintReadOnly)
	float Health = 30.f;

	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY()
	ACombatManager* CombatManager;
};
