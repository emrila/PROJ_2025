#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Shield.generated.h"

class UShieldAttackComp;
class APlayerCharacterBase;
class UStaticMeshComponent;
class UBoxComponent;

DECLARE_LOG_CATEGORY_EXTERN(ShieldLog, Log, All);

UCLASS()
class PROJ_2025_API AShield : public AActor
{
	GENERATED_BODY()

public:
	AShield();
	
	void ActivateShield();
	
	void DeactivateShield();
	
	void SetOwnerProperties(APlayerCharacterBase* NewOwnerCharacter, UShieldAttackComp* NewOwnerAttackComponent, const float NewDurability);
	
	void SetValuesPreActivation(const float NewDamageAmount, const float NewRecoveryRate);

	float GetDamageAmount() const { return DamageAmount; }
	float GetDurability() const { return Durability; }

protected:
	virtual void BeginPlay() override;

	virtual void TickRecovery();
	
	UFUNCTION()
	virtual void OnShieldHit(
		UPrimitiveComponent* HitComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, 
		const FHitResult& Hit
		);
	
	UFUNCTION()
	virtual float TakeDamage(
		float NewDamageAmount, 
		struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator,
		AActor* DamageCauser
		) override;
	
	void ChangeDurability(bool bIncrease, const float AmountToChange);

	virtual void ResetShouldGiveDamage();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ShieldMesh;
	
	float DamageAmount = 20.0f;
	float Durability = 10.f;
	float RecoveryRate = 1.f;
	
	UPROPERTY()
	APlayerCharacterBase* OwnerCharacter;
	
	UPROPERTY()
	UShieldAttackComp* OwnerAttackComponent;
	
	FTimerHandle RecoveryTimerHandle;
	
	bool bShouldGiveDamage = true;
};
