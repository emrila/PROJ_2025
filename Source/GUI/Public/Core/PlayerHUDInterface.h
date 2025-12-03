// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerHUDInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable, BlueprintType)
class UPlayerHUDInterface : public UInterface
{
	GENERATED_BODY()
};


class GUI_API IPlayerHUDInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void InitWidgets();
	virtual void InitWidgets_Implementation(){};	
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ToggleWidget(int32 WidgetGroup = 0);
	virtual void ToggleWidget_Implementation(int32 WidgetGroup){};
};
