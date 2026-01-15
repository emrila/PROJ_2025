// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Selectable.h"
#include "SelectionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SELECTION_API USelectionComponent : public UActorComponent, public ISelectable
{
	GENERATED_BODY()

public:
	USelectionComponent();

	virtual void BeginPlay() override;

protected:
	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Server_OnRegisterSelectable(FGameplayEventData Payload);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Server_OnUnregisterSelectable(FGameplayEventData Payload);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Server_OnRequestSelection(FGameplayEventData Payload);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	virtual void Server_OnValidation(FGameplayEventData Payload);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag SelectionTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag SelectTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag DeselectTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag ValidationTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag LockTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag ConflictTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag ConflictResolvedTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag RegistrationTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Selection")
	FGameplayTag UnregistrationTag;

protected:
	UPROPERTY()
	TArray<class UAsync_WaitGameplayEvent*> WaitGameplayEvents;
};
