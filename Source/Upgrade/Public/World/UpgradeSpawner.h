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
	FUpgradeAlternativePair(AUpgradeAlternative* InAlternative, const FUpgradeDisplayData& InUpgradeData, const int32 NumberOfSpawnAlternatives) : Alternative(InAlternative), UpgradeData(InUpgradeData)
	{
		FocusedByPlayers.SetNum(NumberOfSpawnAlternatives);
		SelectedByPlayers.SetNum(NumberOfSpawnAlternatives);
		LockedForPlayer.SetNum(NumberOfSpawnAlternatives);		
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AUpgradeAlternative* Alternative = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUpgradeDisplayData UpgradeData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<bool> FocusedByPlayers = {false, false, false};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	TArray<bool> SelectedByPlayers = {false, false, false};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<bool> LockedForPlayer = {false, false, false};
};

UCLASS()
class UPGRADE_API AUpgradeSpawner : public AActor, public IInteractable
{
	GENERATED_BODY()


public:	
	AUpgradeSpawner();

	UFUNCTION()
	void OnUpgradeCompleted();
	
	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void TriggerSpawn();

	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void ShowAllUpgradeAlternatives(TArray<FUpgradeAlternativePair> InAssignableUpgrades);

	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void SetNumberOfSpawnAlternatives(const int32 InNumberOfSpawnAlternatives)
	{
		NumberOfSpawnAlternatives = InNumberOfSpawnAlternatives;
	}
	
	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void SetTotalUpgradeNeededForCompletion(const int32 InTotalUpgradeNeededForCompletion)
	{
		TotalUpgradeNeededForCompletion = InTotalUpgradeNeededForCompletion;
	}

	UFUNCTION(Server, Reliable)
	void Server_Spawn();	
	TArray<FUpgradeAlternativePair>& GetUpgradeAlternativePairs() { return UpgradeAlternativePairs; }
	
protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void LockUpgradeAlternatives();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<UStaticMeshComponent> MeshComponent; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<USplineComponent> SpawnSplineComponent;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true))
	int32 NumberOfSpawnAlternatives = 3;
	
	UPROPERTY(Replicated,EditAnywhere, BlueprintReadWrite, Category = "Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true, ExposeOnSpawn=true))
   	bool bSpawnOnBeginPlay = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true))
	TSoftClassPtr<AUpgradeAlternative> UpgradeAlternativeClass;

	UPROPERTY(Replicated, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true, TitleProperty="RowName"))
	TArray<FUpgradeDisplayData> UpgradeDataArray = {};
	
	UPROPERTY(ReplicatedUsing=OnRep_UpgradeAlternativePairs)
	TArray<FUpgradeAlternativePair> UpgradeAlternativePairs;
	
	UFUNCTION()
	void OnRep_UpgradeAlternativePairs();

public:
	virtual void Tick(float DeltaSeconds) override;
	
	virtual void OnInteract_Implementation(UObject* Interactor = nullptr) override;
	virtual bool CanInteract_Implementation() override;

private:	
	
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Upgrade Spawner|Completion", meta=(AllowPrivateAccess=true))
	int32 TotalUpgradeNeededForCompletion = 3;
	
public:
	UPROPERTY(BlueprintAssignable, Category="Upgrade Spawner|Events", meta=(AllowPrivateAccess=true))
	FOnUpgradeEvent OnCompletedAllUpgrades;
};
