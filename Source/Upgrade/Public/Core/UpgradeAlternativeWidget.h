// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "UpgradeDisplayData.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeAlternativeWidget.generated.h"

UCLASS()
class UPGRADE_API UUpgradeAlternativeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnSetUpgradeDisplayData(const FUpgradeDisplayData& Data);
	void OnSetUpgradeDisplayData_Implementation(const FUpgradeDisplayData& Data){};
};
