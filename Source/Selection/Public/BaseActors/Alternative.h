// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Data/SelectablesInfo.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Interactable.h"
#include "Interfaces/Selectable.h"
#include "Interfaces/SelectionDisplayInterface.h"
#include "Alternative.generated.h"

class UInteractableComponent;
class USelectableComponent;
class UAbilitySystemComponent;
class UAsyncWaitGameplayEvent;

UCLASS()
class SELECTION_API AAlternative : public AActor, public IAbilitySystemInterface, public IInteractable, public ISelectable, public ISelectionDisplayInterface
{
	GENERATED_BODY()

public:
	AAlternative();
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void OnInteract_Implementation(UObject* Interactor) override;

	virtual void OnSetSelectablesInfo_Implementation(FInstancedStruct Data) override;
	virtual void OnClearSelectablesInfo_Implementation() override;
	virtual FInstancedStruct OnGetSelectablesInfo_Implementation(const UObject* Selectable = nullptr) override;

protected:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category= "Ability System")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USelectableComponent> SelectableComponent;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UInteractableComponent> InteractableComponent;

	UPROPERTY()
	TArray<class UAsync_WaitGameplayEvent*> WaitGameplayEvents;

	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_SelectablesInfo, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	FSelectablesInfo SelectablesInfo;

	UFUNCTION()
	void OnRep_SelectablesInfo();

};
