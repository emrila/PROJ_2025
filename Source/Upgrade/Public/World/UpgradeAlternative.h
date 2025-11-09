// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "../Core/UpgradeDisplayData.h"
#include "GameFramework/Actor.h"
#include "UpgradeAlternative.generated.h"

class USphereComponent;
class UWidgetComponent;

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUpgrade);

UCLASS()
class UPGRADE_API AUpgradeAlternative : public AActor
{
	GENERATED_BODY()

public:	
	AUpgradeAlternative();	

	UFUNCTION(BlueprintCallable)
	void SetUpgradeDisplayData(const FUpgradeDisplayData& Data);

	UFUNCTION(BlueprintCallable)
	void ShowUpgradeDisplayData() const;

	void NotifyUpgradeSelected() const;

protected:
	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Tick(float DeltaTime) override;	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:	
	bool IsTargetPlayer(const AActor* OtherActor) const;

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative", meta=(AllowPrivateAccess=true))
	TObjectPtr<UWidgetComponent> UpgradeWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative", meta=(AllowPrivateAccess=true))
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative", meta=(AllowPrivateAccess=true))
	TObjectPtr<USphereComponent> UpgradeTriggerComponent;	
	
	UPROPERTY(EditAnywhere, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	float InterpSpeed;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	FUpgradeDisplayData UpgradeDisplayData;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_NotifyUpgradeSelected(bool bInUpgradeSelected);

	UPROPERTY(ReplicatedUsing=OnRep_UpgradeSelected, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	bool bUpgradeSelected;
	UFUNCTION()
	void OnRep_UpgradeSelected();

public:
	//TODO: Explicit stages of upgrade process (selected, applied, etc)?	
	UPROPERTY(BlueprintAssignable, Category="Upgrade Alternative")
	FOnUpgrade OnUpgrade;	
};
