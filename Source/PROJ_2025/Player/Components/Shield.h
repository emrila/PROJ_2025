
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
	
	virtual void ActivateShield();
	
	virtual void DeactivateShield();
	
	void SetDamageAmount(const float Value) { DamageAmount = Value; }
	
	void SetOwnerCharacter(APlayerCharacterBase* NewOwnerCharacter) { OwnerCharacter = NewOwnerCharacter; }

	APlayerCharacterBase* GetOwnerCharacter() const { return OwnerCharacter; }
	
	void SetDurability(const float Value) { Durability = Value; }

protected:
	virtual void BeginPlay() override;
	
	//virtual void TickDurability();

	UFUNCTION()
	virtual void OnShieldOverlap(
		UPrimitiveComponent* OverlappedComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult
		);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ShieldMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBoxComponent* CollisionBox;
	
	float DistanceFromPlayer = 80.f;
	
	float VerticalOffset = 50.f;
	
	float DamageAmount = 20.0f;
	
	float Durability = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KnockbackForce = 1000.f;
	
	UPROPERTY()
	APlayerCharacterBase* OwnerCharacter;
	
	FTimerHandle DurabilityTimerHandle;
	FTimerHandle RecoveryTimerHandle;
	
	bool bCanBeActivated = true;
};
