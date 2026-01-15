// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class INTERACT_API IInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void OnInteract(UObject* Interactor = nullptr);
	virtual void OnInteract_Implementation(UObject* Interactor = nullptr){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	bool CanInteract();
	virtual bool CanInteract_Implementation()
	{
		return true;
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void OnInteractableRangeChanged(FInstancedStruct Payload);
	virtual void OnInteractableRangeChanged_Implementation(FInstancedStruct Payload){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void OnEnterInteractableRange(FInstancedStruct Payload);
	virtual void OnEnterInteractableRange_Implementation(FInstancedStruct Payload){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void OnExitInteractableRange(FInstancedStruct Payload);
	virtual void OnExitInteractableRange_Implementation(FInstancedStruct Payload){}

};
