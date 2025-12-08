#pragma once

#include "CoreMinimal.h"
#include "../AttackComponentBase.h"
#include "RangeAttackComp.generated.h"

struct FInputActionInstance;
class AMageProjectile;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API URangeAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	URangeAttackComp();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void StartAttack() override;
	
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	
	virtual void OnStartAttack(const FInputActionInstance& Instance);
	
	virtual void OnAttackEnd(const FInputActionInstance& Instance);
	
	virtual void OnAttackCanceled(const FInputActionInstance& Instance);
	
	virtual void PerformAttack() override;

	UFUNCTION(Server, Reliable)
	virtual void Server_SpawnProjectile(const FTransform SpawnTransform);
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_SpawnProjectile(const FTransform SpawnTransform);

	virtual FTransform GetProjectileTransform();

	virtual float GetAttackCooldown() const override;

	virtual float GetDamageAmount() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMageProjectile> ProjectileClass;
	
	UPROPERTY(Replicated)
	AMageProjectile* ProjectileInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileOffsetDistanceInFront = 120.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* AttackAnimation;
	
	bool bIsAttacking = false;
};
