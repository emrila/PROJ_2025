// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactor.generated.h"

UDELEGATE(BlueprintType)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFinishedInteraction);

// This class does not need to be modified.
UINTERFACE()
class UInteractor : public UInterface
{
	GENERATED_BODY()
};

class INTERACT_API IInteractor
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFinishedInteraction(const UObject* Interactable);
	virtual void OnFinishedInteraction_Implementation(const UObject* Interactable);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsInteracting() const;
	virtual bool IsInteracting_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	APlayerController* GetLocalPlayerController() const;
	virtual APlayerController* GetLocalPlayerController_Implementation() const;
};
