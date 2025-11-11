// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interact.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInteract : public UInterface
{
	GENERATED_BODY()
};

class UPGRADE_API IInteract
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category="Interactable" )
	void OnInteract(UObject* Interactor);
	virtual void OnInteract_Implementation(UObject* Interactor){}
	
	UFUNCTION( BlueprintNativeEvent, BlueprintCallable, Category="Interactable" )
	bool CanInteract(UObject* Interactor);
	virtual bool CanInteract_Implementation(UObject* Interactor)
	{
		return true;
	}
};
