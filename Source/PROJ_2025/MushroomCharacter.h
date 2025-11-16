// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "MushroomCharacter.generated.h"

UCLASS()
class PROJ_2025_API AMushroomCharacter : public AEnemyBase
{
	GENERATED_BODY()

public:
	AMushroomCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Multicast_Jump(float Angle, FRotator RotationToPlayer, float JumpStrength, float ForwardStrength);
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool AttackIsOnCooldown = false;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsAttacking;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool Bounce;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsRunning = false;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool SlowRunning = false;

	virtual void HandleHit() override;

};
