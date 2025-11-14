// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "../../Core/UpgradeDisplayData.h"
#include "Blueprint/UserWidget.h"
#include "UpgradeAlternativeWidget.generated.h"

UCLASS()
class UPGRADE_API UUpgradeAlternativeWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnSetUpgradeDisplayData(const FUpgradeDisplayData& Data);
	void OnSetUpgradeDisplayData_Implementation(const FUpgradeDisplayData& Data){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnUpgradeSelected(bool bIsSelected);
	void OnUpgradeSelected_Implementation(bool bIsSelected){}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnUpgradeHasFocus(bool bIsSelected);
	void OnUpgradeHasFocus_Implementation(bool bIsSelected){}
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnAlternativeStatusChanged(EUpgradeSelectionStatus Status);
	void OnAlternativeStatusChanged_Implementation(EUpgradeSelectionStatus Status){}

};
