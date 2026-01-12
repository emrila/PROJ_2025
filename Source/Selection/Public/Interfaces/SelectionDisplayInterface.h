// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Interface.h"
#include "SelectionDisplayInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USelectionDisplayInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SELECTION_API ISelectionDisplayInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SelectionDisplay")
	void OnSetSelectablesInfo(FInstancedStruct Data);
	virtual void OnSetSelectablesInfo_Implementation(FInstancedStruct Data){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SelectionDisplay")
	void OnClearSelectablesInfo();
	virtual void OnClearSelectablesInfo_Implementation(){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SelectionDisplay")
	void OnProcessSelectablesInfo();
	virtual void OnProcessSelectablesInfo_Implementation(){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SelectionDisplay")
	FInstancedStruct OnGetSelectablesInfo(const UObject* Selectable = nullptr);
	virtual FInstancedStruct OnGetSelectablesInfo_Implementation(const UObject* Selectable = nullptr)
	{
		return FInstancedStruct();
	}
};
