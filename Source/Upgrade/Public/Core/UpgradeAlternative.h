// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "UpgradeDisplayData.h"
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
	void SetUpgradeDisplayData(const FUpgradeDisplayData& Data) const;

protected:
	UFUNCTION()
	void OnUpgradeTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Tick(float DeltaTime) override;	

private:	
	bool IsTargetPlayer(const AActor* OtherActor) const;

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> UpgradeWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> UpgradeTriggerComponent;	
	
	UPROPERTY(EditAnywhere, Category = "Upgrade Alternative", meta=(AllowPrivateAccess="true"))
	float InterpSpeed;
		
public:
	//TODO: Explicit stages of upgrade process (selected, applied, etc)?
	UPROPERTY(BlueprintAssignable, Category="Upgrade Alternative")
	FOnUpgrade OnUpgrade;
};
