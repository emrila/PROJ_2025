// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "Components/ActorComponent.h"
#include "InteractorComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INTERACT_API UInteractorComponent : public UActorComponent, public IInteractable
{
	GENERATED_BODY()

public:
	UInteractorComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Interaction|Interactor")
	void TraceForInteractable();

	UFUNCTION(BlueprintCallable, Category="Interaction|Interactor")
	void ClearInteractable();

	virtual void OnInteract_Implementation(UObject* Interactor) override;
	virtual bool CanInteract_Implementation() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Getter, Setter, Category="Interact|Trace")
	float InteractionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Getter, Setter, Category="Interact|Trace")
	float InteractionDistance;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Interact|Interactor")
	bool bInteracting;

	UPROPERTY(BlueprintReadOnly, Category="Interact|Interactor")
	TScriptInterface<IInteractable> TargetInteractable;

public:
	UFUNCTION(BlueprintCallable, Category="Interact|Interactor")
	void SetInteracting(const bool bInInteracting)
	{
		bInteracting = bInInteracting;
	}

	UFUNCTION(BlueprintCallable, Category="Interact|Interactor")
	void SetTargetInteractable(const TScriptInterface<IInteractable> InTargetInteractable);

	UFUNCTION(BlueprintPure, Category="Interact|Interactor")
	TScriptInterface<IInteractable> GetTargetInteractable() const
	{
		return TargetInteractable;
	}

	UFUNCTION(BlueprintCallable, Category="Interact|Trace")
	void SetInteractionRadius(const float NewRadius)
	{
		InteractionRadius = NewRadius;
	}

	UFUNCTION(BlueprintPure, Category="Interact|Trace")
	float GetInteractionRadius() const
	{
		return InteractionRadius;
	}

	UFUNCTION(BlueprintCallable, Category="Interact|Trace")
	void SetInteractionDistance(const float NewDistance)
	{
		InteractionDistance = NewDistance;
	}

	UFUNCTION(BlueprintPure, Category="Interact|Trace")
	float GetInteractionDistance() const
	{
		return InteractionDistance;
	}
};
