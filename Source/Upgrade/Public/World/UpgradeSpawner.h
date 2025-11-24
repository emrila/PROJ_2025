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

	UFUNCTION(Server, Reliable)
	void Server_Spawn();	

#if WITH_EDITOR
	virtual void PostLoad() override;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UFUNCTION()
	void LockUpgradeAlternatives();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<UStaticMeshComponent> MeshComponent; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<USplineComponent> SpawnSplineComponent;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true))
	int32 NumberOfSpawnAlternatives = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true))
	TSoftClassPtr<AUpgradeAlternative> UpgradeAlternativeClass;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true))
	FPlayerUpgradeDisplayEntry PlayerUpgradeDisplayEntry;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true, TitleProperty="RowName"))
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
	UPROPERTY(Replicated,EditAnywhere, BlueprintReadWrite, Category = "Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true, ExposeOnSpawn=true))
	bool bSpawnOnBeginPlay = true;
	
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Upgrade Spawner|Completion", meta=(AllowPrivateAccess=true))
	int32 TotalUpgradeNeededForCompletion = 3;
	
public:
	UPROPERTY(BlueprintAssignable, Category="Upgrade Spawner|Events", meta=(AllowPrivateAccess=true))
	FOnUpgradeEvent OnCompletedAllUpgrades;

};
