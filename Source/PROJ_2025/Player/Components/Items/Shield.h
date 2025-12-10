#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shield.generated.h"

class APlayerCharacterBase;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class PROJ_2025_API AShield : public AActor
{
	GENERATED_BODY()

public:
	AShield();
	
	virtual void Tick(float DeltaTime) override;

	virtual void RequestActivateShield();

	UFUNCTION(Server, Reliable)
	virtual void Server_ActivateShield();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_ActivateShield();
	
	virtual void RequestDeactivateShield();

	UFUNCTION(Server, Reliable)
	virtual void Server_DeactivateShield();

	UFUNCTION(NetMulticast, Reliable)
	virtual void Multicast_DeactivateShield();
	
	void SetDamageAmount(const float Value) { DamageAmount = Value; }

	float GetDamageAmount() const { return DamageAmount; }

	void SetDurability(const float Value) { Durability = Value; }

	void SetRecoveryRate(const float Value) { RecoveryRate = Value; }
	
	void ChangeDurability(bool bIncrease, const float AmountToChange);
	
	UFUNCTION(Server, Reliable)
	void Server_BroadcastDurability(const float NewDurability);
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastDurability(const float NewDurability);
	
	void SetOwnerCharacter(APlayerCharacterBase* NewOwnerCharacter);

	APlayerCharacterBase* GetOwnerCharacter() const { return OwnerCharacter; }
	
	UFUNCTION()
	virtual float TakeDamage(
		float NewDamageAmount, 
		struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator,
		AActor* DamageCauser
		) override;

protected:
	virtual void BeginPlay() override;

	virtual void TickDurability();

	virtual void TickRecovery();
	
	UFUNCTION()
	virtual void OnShieldHit(
		UPrimitiveComponent* HitComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, 
		const FHitResult& Hit
		);

	virtual void ResetShouldGiveDamage();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	UStaticMeshComponent* ShieldMesh;
	
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* CollisionBox;*/
	
	UPROPERTY(Replicated)
	float DamageAmount = 20.0f;

	UPROPERTY(Replicated)
	float Durability = 10.f;

	UPROPERTY(Replicated)
	float RecoveryRate = 1.f;
	
	UPROPERTY(Replicated)
	APlayerCharacterBase* OwnerCharacter;
	
	FTimerHandle RecoveryTimerHandle;

	UPROPERTY(Replicated)
	bool bShouldGiveDamage = true;
	
	UPROPERTY(Replicated)
	bool bIsShieldActive = false;
};
