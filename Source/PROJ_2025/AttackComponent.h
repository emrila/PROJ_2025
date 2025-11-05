
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponent.generated.h"


class AMageProjectile;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponent();

	void StartAttack();

protected:
	virtual void BeginPlay() override;

	bool bCanAttack = true;

	bool GetCanAttack() const { return bCanAttack; }

	bool SetCanAttack(const bool bNewCanAttack) { bCanAttack = bNewCanAttack; return bCanAttack; }


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackCoolDown = 3.f;

	FTimerHandle AttackCoolDownTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMageProjectile> ProjectileClass;

	UPROPERTY()
	ACharacter* OwnerCharacter;

	void ResetAttackCooldown();
	
};
