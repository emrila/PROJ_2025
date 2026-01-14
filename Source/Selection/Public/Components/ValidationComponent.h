// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "SelectionComponent.h"
#include "Data/SelectableContainerData.h"
#include "Data/SelectablesInfo.h"
#include "Data/SelectorContainerData.h"
#include "Data/ValidationData.h"
#include "Timers/TaggedEvaluationTimer.h"
#include "ValidationComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SELECTION_API UValidationComponent : public USelectionComponent
{
	GENERATED_BODY()

public:
	UValidationComponent();

	UFUNCTION(BlueprintPure)
	TArray<FSelectablesInfo> GetAllSelectablesInfo() const;

	UFUNCTION(BlueprintPure)
	FSelectablesInfo GetSelectableInfo(const UObject* Selectable) const;

	UFUNCTION(BlueprintPure)
	TArray<FSelectablesInfo> GetSelectableInfoForSelector(const UObject* Selector) const;

	UFUNCTION(Server, Reliable)
	void Server_SetTotalExpectedSelections(int32 InTotalExpectedSelections = -1);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRegisterSelectable_Implementation(FInstancedStruct RegistrationData) override;
	virtual void OnUnregisterSelectable_Implementation(FInstancedStruct UnregistrationData) override;

	virtual void OnRequestSelection_Implementation(FInstancedStruct RequestData) override;

	virtual void OnSelected_Implementation(FInstancedStruct Data) override;
	virtual void OnDeselected_Implementation(FInstancedStruct Data) override;
	virtual void OnValidation_Implementation(FInstancedStruct Data) override;

private:
	AActor* PreProcessSelectableFromRegistrationEvent(const FInstancedStruct& RegistrationData);
	void PostProcessRegistrationEvent();
	void PostProcessSelectionEvent();

	// Timer: Checks Waiting and pending
	UFUNCTION(Server, Reliable)
	void Server_ProcessValidationTransition();
	bool ProcessedWaitingValidations(FGameplayTag EvaluationTag);
	bool ProcessedPendingValidations(FGameplayTag EvaluationTag);

	// Timer: Checks completions
	UFUNCTION(Server, Reliable)
	void Server_ProcessValidationTimer();

	// When completed
	UFUNCTION(Server, Reliable)
	void Server_CompleteValidation(FInstancedStruct CompletionData);
	void CompleteLockValidation();
	void CompleteConflictValidation();

	//Helpers

	FGameplayTag GetValidationTag();

	// Using GameState's player count to determine expected selections as fallback if dynamic count is not set
	int32 GetTotalExpectedSelections() const;

	void OnClearAllValidationFlags(UObject* Selectable = nullptr);
	void OnSetSelectionInfo(UObject* Selectable = nullptr) const;

	void OnApplyValidationEffect(const FGameplayTag Tag, UObject* Selectable = nullptr);
	void OnApplySelectionEffectToSelector(const FGameplayTag Tag, UObject* Selector = nullptr);
	void OnApplySelectionEffectToSelectable(const FGameplayTag Tag, UObject* Selectable = nullptr);


	void SendEventToOwner(AActor* PayloadInstigator, AActor* PayloadTarget, const FGameplayTag EventTag) const;
	void SendEventToSelectable(AActor* PayloadInstigator, AActor* PayloadTarget, const FGameplayTag EventTag, AActor* Selectable) const;

protected:
	UPROPERTY(Replicated)
	FPlayerSelectionContainer SelectionData;

	UPROPERTY(Replicated)
	FSelectablesContainer SelectablesData;


	UPROPERTY(Replicated, BlueprintReadOnly)
	FTaggedEvaluationTimer SelectionEvaluation;

	UPROPERTY()
	FTimerHandle ValidationTransitionTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, FValidationData> ValidationDataMap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, TSubclassOf<UGameplayEffect>> SelectionEffectMap;

	//Selection rules
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxSimultaneousSelections = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAllowMultipleSelectionsPerSelector = false;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAllowMultipleSelectionsOnSameSelectable = false;*/

	// Deselection rules
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAllowDeselection = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition="bAllowDeselection", EditConditionHides = true))
	bool bMustDeselectBeforeNewSelection = false;

	// Completion rules
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAllowDynamicExpectedSelectionCount = false;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly,  meta=(EditCondition="bAllowDynamicExpectedSelectionCount", EditConditionHides = true))
	int32 DynamicExpectedSelectionCount = -1;
};
