// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "../Core/UpgradeDisplayData.h"
#include "Core/UpgradeEvents.h"
#include "GameFramework/Actor.h"
#include "UpgradeAlternative.generated.h"

class USphereComponent;
class UWidgetComponent;

UCLASS()
class UPGRADE_API AUpgradeAlternative : public AActor, public IInteractable
{
	GENERATED_BODY()
	friend class AUpgradeSpawner;
	
public:	
	AUpgradeAlternative();	

	UFUNCTION(BlueprintCallable)
	void SetUpgradeDisplayData(const FUpgradeDisplayData& Data);

	virtual void OnInteract_Implementation(UObject* Interactor) override;
	virtual bool CanInteract_Implementation() override;
	virtual void OnPostInteract_Implementation() override;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	
	virtual void Tick(float DeltaTime) override;	

	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void SelectUpgrade();		
	void SetLocked(bool bToggle);
	void SetFocus(bool bToggle);
	
/*private:	
	bool IsTargetLocalPlayer(const AActor* OtherActor) const;*/
public:	
	UPROPERTY(BlueprintAssignable, Category="Upgrade Alternative")
	FOnUpgrade OnUpgrade;	
	
	UPROPERTY(BlueprintAssignable, Category="Upgrade Alternative")
   	FOnUpgradeEvent OnPostUpgrade;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Upgrade Alternative|Movement", meta=(AllowPrivateAccess=true))
	float InterpSpeed;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	FUpgradeDisplayData UpgradeDisplayData;

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	int32 Index = -1;
	
	UPROPERTY(BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	bool bFocus = false;
	
	UPROPERTY(BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	bool bLocked = false;

	UPROPERTY(ReplicatedUsing=OnRep_Selected, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	bool bSelected = false;	
	
	UFUNCTION()
	void OnRep_Selected();	
	
private:
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
    	TObjectPtr<USceneComponent> SceneComponent;
    	
    	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
    	TObjectPtr<UWidgetComponent> WidgetComponent;
    
    	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
    	TObjectPtr<USphereComponent> SphereComponent;	
    	
};
