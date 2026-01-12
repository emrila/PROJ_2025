// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "BaseActors/Alternative.h"
#include "Data/SelectablesInfo.h"
#include "Data/UpgradeDisplayData.h"
#include "GameFramework/Actor.h"
#include "Interfaces/UpgradeDisplayInterface.h"
#include "UpgradeAlternative.generated.h"

class USphereComponent;
class UWidgetComponent;

UCLASS()
class UPGRADE_API AUpgradeAlternative : public AAlternative, public IUpgradeDisplayInterface
{
	GENERATED_BODY()

public:
	AUpgradeAlternative();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

public:
	virtual void OnSetUpgradeDisplayData_Implementation(FInstancedStruct Data) override;
	virtual void OnClearUpgradeDisplayData_Implementation() override;
	virtual FInstancedStruct OnGetUpgradeDisplayData_Implementation() override;
	virtual void OnProcessUpgradeDisplayData_Implementation() override;


	virtual void OnProcessSelectablesInfo_Implementation() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Upgrade Alternative|Movement", meta=(AllowPrivateAccess=true))
	float InterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<USceneComponent> SceneComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;


	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_UpgradeDisplayData, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	FUpgradeDisplayData UpgradeDisplayData;
	UFUNCTION()
	void OnRep_UpgradeDisplayData();

};
