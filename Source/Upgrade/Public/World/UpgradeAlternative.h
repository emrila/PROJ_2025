// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Interact.h"
#include "Interactable.h"
#include "../Core/UpgradeDisplayData.h"
#include "GameFramework/Actor.h"
#include "UpgradeAlternative.generated.h"

class USphereComponent;
class UWidgetComponent;

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgrade, FUpgradeDisplayData, SelectedUpgrade);

UENUM(BlueprintType)
enum class EUpgradeSelectionStatus: uint8
{
	NotSelected		UMETA(DisplayName = "Not Selected"),
	Locked			UMETA(DisplayName = "Locked"),
	Hovered			UMETA(DisplayName = "Hovered"),
	Selected		UMETA(DisplayName = "Selected")
};

UDELEGATE(Blueprintable)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusChanged, EUpgradeSelectionStatus, NewStatus, int32, Index);

UCLASS()
class UPGRADE_API AUpgradeAlternative : public AActor, public IInteractable
{
	GENERATED_BODY()

public:	
	AUpgradeAlternative();	

	UFUNCTION(BlueprintCallable)
	void SetUpgradeDisplayData(const FUpgradeDisplayData& Data);

	void NotifyUpgradeSelected();

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

	//void SetCurrentSelectionStatus(const EUpgradeSelectionStatus NewStatus);
	/*UPROPERTY(Replicated, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	EUpgradeSelectionStatus CurrentSelectionStatus = EUpgradeSelectionStatus::NotSelected;*/

	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	int32 Index = -1;
	
	UPROPERTY(BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	bool bFocus = false;
	
	UPROPERTY(ReplicatedUsing=OnRep_UpgradeSelected, BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	bool bSelected = false;	
	
	UPROPERTY(BlueprintReadWrite, Category = "Upgrade Alternative", meta=(AllowPrivateAccess=true))
	bool bLocked = false;
	
	UFUNCTION()
	void OnRep_UpgradeSelected();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_NotifyUpgradeSelected(bool bInUpgradeSelected);	
	
public:
	virtual void OnInteract_Implementation(UObject* Interactor) override;
	virtual bool CanInteract_Implementation() override;

protected:
	friend class AUpgradeSpawner;
	
public:
	UPROPERTY(BlueprintAssignable, Category="Upgrade Alternative")
	FOnUpgrade OnUpgrade;
	
	UPROPERTY(BlueprintAssignable, Category="Upgrade Alternative")
	FOnStatusChanged OnStatusChanged;
};
