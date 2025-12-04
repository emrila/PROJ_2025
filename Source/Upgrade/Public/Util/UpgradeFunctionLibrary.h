// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UpgradeFunctionLibrary.generated.h"


class AUpgradeSpawner;
class UUpgradeComponent;

UCLASS()
class UPGRADE_API UUpgradeFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintCallable, Category="Upgrade|Util", meta=(WorldContext="WorldContextObject"))
	static AUpgradeSpawner* GetLocalUpgradeSpawner(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="Upgrade|Util", meta=(WorldContext="WorldContextObject"))
	static UUpgradeComponent* GetLocalUpgradeComponent(UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category="Upgrade|Util", meta=(WorldContext="WorldContextObject"))
	static bool IsLocalPlayer(const AActor* OtherActor);
};
