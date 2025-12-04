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

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCanRecast);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class PROJ_2025_API UAttackComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponentBase();
	
	virtual void StartAttack();
	
	virtual void StartAttack(const float NewDamageAmount, float NewAttackCooldown);

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
	
	UPROPERTY(BlueprintAssignable)
	FOnCanRecast OnCanRecast;

protected:
	virtual void BeginPlay() override;

	virtual void PerformAttack() {}

	//UFUNCTION(Server, Reliable)
	virtual void ResetAttackCooldown();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	APlayerCharacterBase* OwnerCharacter;
	
	bool bCanAttack = true;
	
	float DamageAmount = 10.0f;
	
	float DamageAmountToStore= 0.f;
	
	float AttackCooldown = 1.f;

	float AttackCooldownToStore = 0.f;

	FTimerHandle AttackCooldownTimerHandle;

	UFUNCTION(NetMulticast, Reliable)
	virtual void SpawnParticles(APlayerCharacterBase* PlayerCharacter, FHitResult Hit);
	
	UPROPERTY(BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	float AttackSpeedModifier = 1.f;
	
	UPROPERTY(BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	float AttackDamageModifier = 1.f;
};
