// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "../Core/UpgradeDisplayData.h"
#include "GameFramework/Actor.h"
#include "UpgradeAlternative.generated.h"

class USphereComponent;
class UWidgetComponent;

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreUpgrade);

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgrade, FUpgradeDisplayData, SelectedUpgrade);

UCLASS()
class UPGRADE_API AUpgradeAlternative : public AActor, public IInteractable
{
	GENERATED_BODY()

public:	
	AUpgradeAlternative();	

	UFUNCTION(BlueprintCallable)
	void SetUpgradeDisplayData(const FUpgradeDisplayData& Data);

protected:
	void SelectUpgrade();

	UFUNCTION()
	void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:	
	bool IsTargetLocalPlayer(const AActor* OtherActor) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<USceneComponent> SceneComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Upgrade Alternative|Components", meta=(AllowPrivateAccess=true))
	TObjectPtr<USphereComponent> SphereComponent;	
	
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
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SelectUpgrade(bool bIsSelected);	
	
public:
	virtual void OnPreInteract_Implementation() override;
	virtual void OnInteract_Implementation(UObject* Interactor) override;
	virtual bool CanInteract_Implementation() override;

protected:
	friend class AUpgradeSpawner;
	
public:
	UPROPERTY(BlueprintAssignable, Category="Upgrade Alternative")
	FOnPreUpgrade OnPreUpgrade;

	UPROPERTY(BlueprintAssignable, Category="Upgrade Alternative")
	FOnUpgrade OnUpgrade;	
};
