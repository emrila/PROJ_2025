// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "UpgradeSpawner.generated.h"

class AUpgradeAlternative;

UCLASS()
class UPGRADE_API AUpgradeSpawner : public AActor
{
	GENERATED_BODY()

public:	
	AUpgradeSpawner();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Upgrade Spawner")
	void OnSpawnAlternatives();
	void OnSpawnAlternatives_Implementation();

protected:	
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnUpgradeSelected();

	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USplineComponent> SpawnSplineComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess="true"))
	int32 NumberOfSpawnAlternatives = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess="true"))
	TSoftClassPtr<AUpgradeAlternative> UpgradeAlternativeClass;
};
