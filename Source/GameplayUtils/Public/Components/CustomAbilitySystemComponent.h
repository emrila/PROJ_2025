// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CustomAbilitySystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEPLAYUTILS_API UCustomAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UCustomAbilitySystemComponent();

	UFUNCTION(BlueprintCallable, Category="Ability System")
	TArray<FGameplayAbilitySpecHandle> GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant);

	UFUNCTION(BlueprintCallable, Category="Ability System")
	void SendAbilitiesChangedEvent();

protected:
	virtual void OnRep_ActivateAbilities() override;

	UPROPERTY()
	TArray<FGameplayAbilitySpec> LastActivatedAbility;
};
