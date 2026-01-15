// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Interface.h"
#include "UpgradeDisplayInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUpgradeDisplayInterface : public UInterface
{
	GENERATED_BODY()
};

class UPGRADE_API IUpgradeDisplayInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UpgradeDisplay")
	void OnSetUpgradeDisplayData(FInstancedStruct Data);
	virtual void OnSetUpgradeDisplayData_Implementation(FInstancedStruct Data){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UpgradeDisplay")
	void OnClearUpgradeDisplayData();
	virtual void OnClearUpgradeDisplayData_Implementation(){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UpgradeDisplay")
	void OnProcessUpgradeDisplayData();
	virtual void OnProcessUpgradeDisplayData_Implementation(){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UpgradeDisplay")
	FInstancedStruct OnGetUpgradeDisplayData();
	virtual FInstancedStruct OnGetUpgradeDisplayData_Implementation()
	{
		return FInstancedStruct();
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UpgradeDisplay")
	void OnSetOwner(AActor* Owner);
	virtual void OnSetOwner_Implementation(AActor* Owner){}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="UpgradeDisplay")
	AActor* OnGetOwner();
	virtual AActor* OnGetOwner_Implementation()
	{
		return nullptr;
	}
};
