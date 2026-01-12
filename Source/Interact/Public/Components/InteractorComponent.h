// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Interactable.h"
#include "Interfaces/Interactor.h"
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

	UFUNCTION()
	void OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION(BlueprintCallable, Category="Interaction|Interactor")
	void TraceForInteractable();
	void OnSetTargetInteractable(const UObject* InTargetInteractable, const UObject* EventTarget, FGameplayTag EventTag);

public:
	UFUNCTION(BlueprintCallable, Category="Interaction|Interactor")
	void ClearInteractable();

	virtual void OnInteract_Implementation(UObject* Interactor) override;
	virtual bool CanInteract_Implementation() override;
	virtual void OnFinishedInteraction_Implementation(const UObject* Interactable) override;

	virtual void OnSendInteractionData_Implementation(FInstancedStruct Data) override;

	UFUNCTION(Server, Reliable)
	void Server_OnInteract();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interact|Trace")
	float InteractionRadius = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Interact|Trace")
	float InteractionDistance = 200.f;;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Interact|Interactor")
	bool bInteracting = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category="Interact|Interactor")
	UObject* TargetInteractableObject = nullptr;

public:
	UFUNCTION(BlueprintCallable, Category="Interact|Interactor")
	void SetInteracting(const bool bInInteracting);
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Interact|Interactor")
	void Server_SetInteracting(const bool bInInteracting);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Interact|Interactor")
	void Server_SetTargetInteractable(const UObject* InTargetInteractable);
	void SetTargetInteractable(UObject* InTargetInteractable);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category="Interact|Interactor")
	void Client_SendProximityEvent(const UObject* InTargetInteractable, const FGameplayTag EventTag);

	UFUNCTION(BlueprintCallable, Category="Interact|Interactor")
	void SendProximityEvent(const UObject* InTargetInteractable, const FGameplayTag EventTag);

	UFUNCTION(BlueprintPure, Category="Interact|Interactor")
	UObject* GetTargetInteractableObject() const
	{
		return TargetInteractableObject;
	}

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interact|Trace")
	TEnumAsByte<EDrawDebugTrace::Type> DebugType = EDrawDebugTrace::None;
#endif

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Interaction")
	FGameplayTag InteractTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Interaction")
	FGameplayTag ProximityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Interaction")
	FGameplayTag EnterTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Interaction")
	FGameplayTag ExitTag;

};
