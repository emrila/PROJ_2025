
#pragma once

#include "CoreMinimal.h"
#include "Player/Components/AttackComponentBase.h"
#include "ShieldAttackComp.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJ_2025_API UShieldAttackComp : public UAttackComponentBase
{
	GENERATED_BODY()

public:
	UShieldAttackComp();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
	
	virtual void SetupOwnerInputBinding(UEnhancedInputComponent* OwnerInputComp, UInputAction* OwnerInputAction) override;
	
	//virtual void StartAttack() override;
	
	//virtual void PerformAttack() override;
	
	virtual void Test();

protected:
	virtual void BeginPlay() override;
	
	bool bIsTest = false;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AShield> ShieldClass;
	
	UPROPERTY()
	AShield* CurrentShield;
	
};
