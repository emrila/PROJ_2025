// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BaseActors/AlternativeManager.h"
#include "Components/SplineComponent.h"
#include "Data/UpgradeDisplayData.h"
#include "Data/UpgradeEvents.h"
#include "GameFramework/Actor.h"
#include "UpgradeSpawner.generated.h"

UCLASS()
class UPGRADE_API AUpgradeSpawner : public AAlternativeManager
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="Upgrade Spawner|Events", meta=(AllowPrivateAccess=true))
	FOnUpgradeEvent OnCompletedAllUpgrades;

	AUpgradeSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void OnValidatedLock(FGameplayEventData Payload);

	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void SetAllUpgradesDisplayData();

	UFUNCTION(Server, Reliable)
	void Server_Spawn();

public:
	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void TriggerSpawn();

	UFUNCTION(BlueprintCallable, Category="Upgrade Spawner")
	void SetTotalUpgradeNeededForCompletion(const int32 InTotalUpgradeNeededForCompletion);

	UFUNCTION(Server, Reliable)
	void Server_ClearAll();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner", meta=(AllowPrivateAccess=true))
	TObjectPtr<USplineComponent> SpawnSplineComponent;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true, ClampMin = 0))
	int32 TotalToSpawn = 3;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true, ExposeOnSpawn=true))
	bool bSpawnOnBeginPlay = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true))
	TSoftClassPtr<AActor> SpawnClass;

	UPROPERTY(ReplicatedUsing=OnRep_UpgradeDataArray, BlueprintReadWrite, Category="Upgrade Spawner|Spawning", meta=(AllowPrivateAccess=true, TitleProperty="RowName"))
	TArray<FUpgradeDisplayData> UpgradeDataArray = {};

	UFUNCTION()
	void OnRep_UpgradeDataArray();

};
