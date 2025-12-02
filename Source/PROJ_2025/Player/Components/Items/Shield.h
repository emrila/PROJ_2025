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
	
	void SetOwnerCharacter(APlayerCharacterBase* NewOwnerCharacter);

	APlayerCharacterBase* GetOwnerCharacter() const { return OwnerCharacter; }

protected:
	virtual void BeginPlay() override;

	virtual void TickDurability();

	virtual void TickRecovery();

	UFUNCTION()
	virtual void OnShieldOverlap(
		UPrimitiveComponent* OverlappedComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult
		);

	virtual void ResetShouldGiveDamage();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ShieldMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* CollisionBox;
	
	float DamageAmount = 20.0f;

	UPROPERTY(Replicated)
	float Durability = 10.f;

	float RecoveryRate = 1.f;
	
	UPROPERTY()
	APlayerCharacterBase* OwnerCharacter;
	
	FTimerHandle DurabilityTimerHandle;
	FTimerHandle RecoveryTimerHandle;

	bool bShouldGiveDamage = true;
};
