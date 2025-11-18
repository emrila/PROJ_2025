// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "UpgradeAlternative.h"
#include "../Core/UpgradeDisplayData.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "UpgradeSpawner.generated.h"

class AUpgradeAlternative;

USTRUCT(BlueprintType)
struct FUpgradeAlternativePair
{
	GENERATED_BODY()

	FUpgradeAlternativePair() = default;
	FUpgradeAlternativePair(AUpgradeAlternative* InAlternative, const FUpgradeDisplayData& InUpgradeData) : Alternative(InAlternative), UpgradeData(InUpgradeData)	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AUpgradeAlternative* Alternative = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUpgradeDisplayData UpgradeData;
};

UCLASS()
class UPGRADE_API AUpgradeSpawner : public AActor
{
	GENERATED_BODY()

public:	
	AUpgradeSpawner();

	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void TriggerSpawn();

	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void ShowAllUpgradeAlternatives(TArray<FUpgradeAlternativePair> InAssignableUpgrades);

	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void SetNumberOfSpawnAlternatives(const int32 InNumberOfSpawnAlternatives)
	{
		NumberOfSpawnAlternatives = InNumberOfSpawnAlternatives;
	}

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Spawn();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnUpgradeSelected(FUpgradeDisplayData SelectedUpgrade);

	UFUNCTION()
	void LockUpgradeAlternatives();


private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<USplineComponent> SpawnSplineComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	int32 NumberOfSpawnAlternatives = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TSoftClassPtr<AUpgradeAlternative> UpgradeAlternativeClass;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	FPlayerUpgradeDisplayEntry PlayerUpgradeDisplayEntry; //TODO: Sync with UpgradeSubsystem

	UPROPERTY(ReplicatedUsing=OnRep_UpgradeAlternativePairs)
	TArray<FUpgradeAlternativePair> UpgradeAlternativePairs;

	UFUNCTION()
	void OnRep_UpgradeAlternativePairs();
	
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Upgrade Spawner", meta=(AllowPrivateAccess=true))
	bool bSpawnOnBeginPlay = true;	

};
