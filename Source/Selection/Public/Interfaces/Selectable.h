// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Interface.h"
#include "Selectable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class USelectable : public UInterface
{
	GENERATED_BODY()
};

class SELECTION_API ISelectable
{
	GENERATED_BODY()

public:
	// REGISTRATION EVENTS
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selectable")
	void OnRegisterSelectable(FInstancedStruct RegistrationData);
	virtual void OnRegisterSelectable_Implementation(FInstancedStruct RegistrationData){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selectable")
	void OnUnregisterSelectable(FInstancedStruct UnregistrationData);
	virtual void OnUnregisterSelectable_Implementation(FInstancedStruct UnregistrationData){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selectable")
	bool CanRegister() const;
	virtual bool CanRegister_Implementation() const{ return true; }

	// REQUEST SELECTION
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selectable")
	void OnRequestSelection(FInstancedStruct RequestData);
	virtual void OnRequestSelection_Implementation(FInstancedStruct RequestData){}

	// SELECTION EVENTS
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selectable")
	void OnSelected(FInstancedStruct Data);
	virtual void OnSelected_Implementation(FInstancedStruct Data){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selectable")
	void OnDeselected(FInstancedStruct Data);
	virtual void OnDeselected_Implementation(FInstancedStruct Data){}

	// VALIDATION EVENTS
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Selectable")
	void OnValidation(FInstancedStruct Data);
	virtual void OnValidation_Implementation(FInstancedStruct Data){}
};
