// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
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
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void OnPreInteract(UObject* Interactor = nullptr);
	virtual void OnPreInteract_Implementation(UObject* Interactor = nullptr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void OnInteract(UObject* Interactor = nullptr);
	virtual void OnInteract_Implementation(UObject* Interactor = nullptr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void OnPostInteract(UObject* Interactor = nullptr);
	virtual void OnPostInteract_Implementation(UObject* Interactor = nullptr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	void SetLocalPlayerController(APlayerController* PlayerController = nullptr);
	virtual void SetLocalPlayerController_Implementation(APlayerController* PlayerController = nullptr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interactable")
	bool CanInteract();
	virtual bool CanInteract_Implementation();


};
