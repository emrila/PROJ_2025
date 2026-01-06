#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttackComponentBase.generated.h"


class UNiagaraSystem;
struct FInputActionInstance;
class APlayerCharacterBase;
class UInputAction;

DECLARE_LOG_CATEGORY_EXTERN(AttackComponentLog, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownStarted, float, CurrentCooldownDuration);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class PROJ_2025_API UAttackComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttackComponentBase();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction);
	
	UFUNCTION(BlueprintCallable)
	virtual float GetCooldownDuration();
	
	UFUNCTION(BlueprintCallable)
	float GetDamageAmount() const;
	
	float GetCurrentAnimationLength() const;
	
	UFUNCTION(BlueprintCallable)
	float GetAttackSpeedModifier() const { return AttackSpeedModifier; }
	
	UFUNCTION(BlueprintCallable)
	float GetAttackDamageModifier() const { return AttackDamageModifier; }
	
	UPROPERTY(BlueprintAssignable)
	FOnCooldownStarted OnCooldownStarted;

protected:
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnPreAttack(const FInputActionInstance& InputActionInstance);
	virtual void OnStartAttack(const FInputActionInstance& InputActionInstance);
	
	virtual void StartAttack();
	virtual void PerformAttack();
	
	void PlayAttackAnimation();
	
	UFUNCTION(Server, Reliable)
	void Server_PlayAttackAnimation();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackAnimation();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnImpactParticles(const FVector& ImpactLocation);
	
	void SetCurrentAnimationIndex();
	
	virtual void Reset();
	
	UPROPERTY()
	APlayerCharacterBase* OwnerCharacter;
	
	FTimerHandle InitialDelayTimerHandle;
	FTimerHandle CooldownTimerHandle;
	FTimerHandle SweepTimerHandle;
	
	bool bCanAttack = true;
	bool bIsAttacking = false;
	bool bIsFirstAttackAnimSet = false;
	
	float AttackSweepRadius = 200.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackCooldown = 0.5f;
	
	UPROPERTY(BlueprintReadOnly)
	float DamageAmount = 10.f;
	
	UPROPERTY(BlueprintReadOnly, Replicated,meta = (AllowPrivateAccess = "true"))
	float AttackSpeedModifier = 1.f;
	
	UPROPERTY(BlueprintReadOnly, Replicated, meta = (AllowPrivateAccess = "true"))
	float AttackDamageModifier = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Attack Animations")
	TArray<UAnimMontage*> AttackAnimations;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	UNiagaraSystem* ImpactParticles;
	
	int32 CurrentAnimIndex = 0;
	
	float CurrentAnimationPlayRate = 1.f;  //Check if this works without replication
	
	bool bDebug = true;
	
	virtual void RequestDebug();
	
	virtual void Debug();
	
	UFUNCTION(Server, Reliable)
	void Server_Debug();
};
