#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponentBase.generated.h"

class UNiagaraSystem;
class UInputAction;
class UEnhancedInputComponent;
class APlayerCharacterBase;

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownTimerStarted, float, CurrentCoolDownTime);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UAttackComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponentBase();
	
	virtual void StartAttack();
	
	virtual void StartAttack(const float NewDamageAmount);

	virtual bool GetCanAttack() const { return bCanAttack; }

	virtual bool SetCanAttack(const bool bNewCanAttack) { bCanAttack = bNewCanAttack; return bCanAttack; }

	virtual void SetDamageAmount(const float Value) { DamageAmount = Value; }

	virtual float GetDamageAmount() const { return DamageAmount; }

	virtual float GetAttackCooldown() const { return AttackCooldown; }

	virtual void SetAttackCooldown(const float Value) { AttackCooldown = Value; }
	
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction);
	
	UFUNCTION(Server, Reliable)
	void Server_SpawnEffect(const FVector& EffectSpawnLocation, UNiagaraSystem* Effect);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnEffect(const FVector& EffectSpawnLocation, UNiagaraSystem* Effect);
	
	UPROPERTY(BlueprintAssignable)
	FOnCooldownTimerStarted OnCooldownTimerStarted;

protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack() {}

	virtual void ResetAttackCooldown();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	ACharacter* OwnerCharacter;
	
	bool bCanAttack = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageAmount = 10.0f;
	
	float DamageAmountToStore= 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackCooldown = 1.f;

	FTimerHandle AttackCooldownTimerHandle;

	UFUNCTION(NetMulticast, Reliable)
	virtual void SpawnParticles(APlayerCharacterBase* PlayerCharacter, FHitResult Hit);
	
	UPROPERTY(BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	float UpgradeMultiplier = 1.f;
};
