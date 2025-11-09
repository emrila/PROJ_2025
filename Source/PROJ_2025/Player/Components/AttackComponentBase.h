#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponentBase.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UAttackComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponentBase();
	
	virtual void StartAttack();

	virtual bool GetCanAttack() const { return bCanAttack; }

	virtual bool SetCanAttack(const bool bNewCanAttack) { bCanAttack = bNewCanAttack; return bCanAttack; }

protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack() {}

	virtual void ResetAttackCooldown();

	UPROPERTY()
	ACharacter* OwnerCharacter;
	
	bool bCanAttack = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackCoolDown = 1.f;

	FTimerHandle AttackCoolDownTimerHandle;
};
