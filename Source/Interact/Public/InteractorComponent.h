// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "Interactor.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "StructUtils/InstancedStruct.h"
#include "InteractorComponent.generated.h"

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFinishedInteractionEvent, FInstancedStruct , InteractionData);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INTERACT_API UInteractorComponent : public UActorComponent, public IInteractable, public IInteractor
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintAssignable, Category="Interaction|Interactor")
	FOnFinishedInteractionEvent OnFinishedInteraction;
	
	UInteractorComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category="Interaction|Interactor")
	void TraceForInteractable();

	UFUNCTION(BlueprintCallable, Category="Interaction|Interactor")
	void ClearInteractable();

	virtual void OnInteract_Implementation(UObject* Interactor) override;
	virtual bool CanInteract_Implementation() override;
	virtual void OnFinishedInteraction_Implementation(const UObject* Interactable) override;
	virtual int32 GetOwnerID_Implementation() const override;
	virtual void OnSuperFinishedInteraction_Implementation(FInstancedStruct InteractionData) override;
	
	UFUNCTION(Server, Reliable)
	void Server_InteractWith(UObject* Interactable);
	
	UFUNCTION(Server, Reliable)
	void Server_SetOwnerID(const int32 InOwnerID);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Getter, Setter, Category="Interact|Trace")
	float InteractionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Getter, Setter, Category="Interact|Trace")
	float InteractionDistance;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Interact|Interactor")
	bool bInteracting;
	
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Interact|Interactor")
	int32 OwnerID = -1; // -1 == Not set

	UPROPERTY(BlueprintReadOnly, Category="Interact|Interactor")
	TScriptInterface<IInteractable> TargetInteractable;

public:
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Interact|Interactor")
	void Server_SetInteracting(const bool bInInteracting);

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



#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact|Trace")
	TEnumAsByte<EDrawDebugTrace::Type> DebugType = EDrawDebugTrace::None;
#endif
};
