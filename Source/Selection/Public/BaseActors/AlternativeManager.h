// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Selectable.h"
#include "Interfaces/SelectionDisplayInterface.h"
#include "AlternativeManager.generated.h"

class UCustomAbilitySystemComponent;
class UAsync_WaitGameplayEvent;
class UValidationComponent;
class UAsyncWaitGameplayEvent;

UCLASS()
class SELECTION_API AAlternativeManager : public AActor, public IAbilitySystemInterface, public ISelectable, public ISelectionDisplayInterface
{
	GENERATED_BODY()

public:
	AAlternativeManager();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION()
	void WrapperOnInteractionEvent(FGameplayEventData Payload);

	virtual FInstancedStruct OnGetSelectablesInfo_Implementation(const UObject* Selectable = nullptr) override;

protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category= "Ability System")
	TObjectPtr<UCustomAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category= "Ability System")
	TObjectPtr<UValidationComponent> ValidationComponent;

	UPROPERTY()
	TArray<UAsync_WaitGameplayEvent*> WaitGameplayEvents;

};
