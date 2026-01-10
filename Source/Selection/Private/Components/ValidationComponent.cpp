// 🐲Furkan approves of this🐲


#include "Components/ValidationComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Async/Async_WaitGameplayEvent.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Utility/SelectionLog.h"
#include "Utility/Tags.h"

UValidationComponent::UValidationComponent()
{
	ValidationDurationPerTag.Add(LockTag, 2.0f);
	ValidationDurationPerTag.Add(ConflictTag, 5.0f);
}

TArray<FSelectablesInfo> UValidationComponent::GetAllSelectablesInfo() const
{
	TArray<FSelectablesInfo> SelectablesInfo;

	for (const auto& Item : SelectablesData.Items)
	{
		FSelectablesInfo Info;
		Info.Selectable = Item.Selectable;
		Info.Selectors = SelectionData.GetSelectablesSelectors(Item.Selectable);
		Info.TotalSelectors = Info.Selectors.Num();
		Info.EvaluationFlags = Item.EvaluationFlags;

		SelectablesInfo.Add(Info);
	}

	return SelectablesInfo;
}

void UValidationComponent::BeginPlay()
{
	Super::BeginPlay();

	WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(GetOwner(), RegistrationTag, false, false))->EventReceived.AddDynamic(
	this, &UValidationComponent::Server_OnRegisterSelectable);

	WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(GetOwner(), UnregistrationTag, false, false))->EventReceived.AddDynamic(
		this, &UValidationComponent::Server_OnUnregisterSelectable);

	if (GetOwner()->HasAuthority())
	{
		GetOwner()->GetWorldTimerManager().SetTimer(ValidationTransitionTimerHandle,
										FTimerDelegate::CreateUObject(this, &UValidationComponent::Server_ProcessValidationTransition), 1.0f, true, 2.0f);
	}
}

void UValidationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UValidationComponent, SelectionData);
	DOREPLIFETIME(UValidationComponent, SelectablesData);
	DOREPLIFETIME(UValidationComponent, SelectionEvaluation);

	DOREPLIFETIME(UValidationComponent, DynamicExpectedSelectionCount);
}

void UValidationComponent::Server_SetTotalExpectedSelections_Implementation(const int32 InTotalExpectedSelections)
{
	if (!bAllowDynamicExpectedSelectionCount)
	{
		return;
	}
	if (InTotalExpectedSelections < 0)
	{
		bAllowDynamicExpectedSelectionCount = false;
		return;
	}
	DynamicExpectedSelectionCount = InTotalExpectedSelections;
}

void UValidationComponent::OnRegisterSelectable_Implementation(FInstancedStruct RegistrationData)
{
	AActor* Selectable = PreProcessRegistrationEvent(RegistrationData);
	if (!Selectable)
	{
		return;
	}

	SELECTION_DISPLAY(TEXT("👨‍👩‍👧‍👦 Registering: %s with: %s as Selectable."), *Selectable->GetName(), *GetName());
	SelectablesData.AddOrUpdate(Selectable, Selectable->GetActorLocation());
	PostProcessRegistrationEvent();
}

void UValidationComponent::OnUnregisterSelectable_Implementation(FInstancedStruct UnregistrationData)
{
	AActor* Selectable = PreProcessRegistrationEvent(UnregistrationData);
	if (!Selectable)
	{
		return;
	}
	SELECTION_DISPLAY(TEXT("👨‍👩‍👧‍👦 Unregistering: %s with: %s as Selectable."), *Selectable->GetName(), *GetName());
	SelectablesData.Remove(Selectable);
	PostProcessRegistrationEvent();
}

void UValidationComponent::OnRequestSelection_Implementation(FInstancedStruct RequestData)
{
	const FGameplayEventData* GameplayEventData = RequestData.GetPtr<FGameplayEventData>();
	if (!GameplayEventData)
	{
		SELECTION_ERROR(TEXT("%hs called with invalid GameplayEventData."), __FUNCTION__);
		return;
	}
	const AActor* Selectable = GameplayEventData->Target.Get();
	const AActor* Selector = GameplayEventData->Instigator.Get();
	if (!Selector || !Selectable)
	{
		SELECTION_ERROR(TEXT("%hs called with invalid Selector or Selectable."), __FUNCTION__);
		return;
	}

	//Keeping check for already selected separate until the possible need for defining custom selection rules is clearer
	if (SelectionData.IsCurrentSelectionSelectedBySelector(Selector, Selectable))
	{
		if (!bAllowDeselection)
		{
			return;
		}
		SELECTION_DISPLAY(TEXT("%hs Selector %s attempted to select already selected Selectable %s. Deselecting now."),__FUNCTION__, *Selector->GetName(), *Selectable->GetName());
		Execute_OnDeselected(this, RequestData);
	}
	else
	{
		SELECTION_DISPLAY(TEXT("%hs Selector %s selecting Selectable %s."),__FUNCTION__, *Selector->GetName(), *Selectable->GetName());
		// Assuming single selection per selector, and new selection should be prioritized -> Deselect previous selection if found
		if (UObject* CurrentSelectable = SelectionData.GetSelectable(Selector))
		{
			if (!bAllowDeselection || bMustDeselectBeforeNewSelection)
			{
				return;
			}

			SELECTION_DISPLAY(TEXT("%hs Selector %s had previous Selectable %s. Deselecting now."),__FUNCTION__, *Selector->GetName(), *CurrentSelectable->GetName());
			FGameplayEventData Payload;
			Payload.Instigator = Selector;
			Payload.Target = Cast<AActor>(CurrentSelectable);
			Execute_OnDeselected(this, FInstancedStruct::Make<FGameplayEventData>(Payload));
		}
		Execute_OnSelected(this, RequestData);
	}

	PostProcessSelectionEvent();
}

void UValidationComponent::OnSelected_Implementation(FInstancedStruct Data)
{
	const FGameplayEventData* GameplayEventData = Data.GetPtr<FGameplayEventData>();
	if (!GetOwner()->HasAuthority() || !GameplayEventData)
	{
		return;
	}
	AActor* Selectable = const_cast<AActor*>(GameplayEventData->Target.Get());
	const AActor* Selector = GameplayEventData->Instigator.Get();
	if (!Selector || !Selectable)
	{
		return;
	}

	// Handle selection
	SelectionData.AddOrUpdate(Selector, Selectable);
	SelectablesData.SetPendingSelectionNotification(Selectable);

	// Handle potential selection conflicts
	if (SelectionData.GetSelectablesSelectors(Selectable).Num() > MaxSimultaneousSelections && !SelectablesData.HasAnyConflict())
	{
		SELECTION_WARNING(TEXT("⚠️ Selectable %s has conflict with multiple selectors."), *Selectable->GetName());
		SelectablesData.SetWaiting(ConflictTag, Selectable);
	}

	SELECTION_DISPLAY(TEXT("🛜 Selector %s selected Selectable %s."), *Selector->GetName(), *Selectable->GetName());

	FGameplayEventData Payload;
	Payload.Instigator = Selector;//GetOwner();
	Payload.Target = Selectable;
	Payload.EventTag = SelectTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Selectable, Payload.EventTag, Payload);
}

void UValidationComponent::OnDeselected_Implementation(FInstancedStruct Data)
{
	const FGameplayEventData* GameplayEventData = Data.GetPtr<FGameplayEventData>();
	if (!GetOwner()->HasAuthority() || !GameplayEventData || !bAllowDeselection)
	{
		return;
	}

	AActor* Selectable = const_cast<AActor*>(GameplayEventData->Target.Get());
	const AActor* Selector = GameplayEventData->Instigator.Get();
	if (!Selector || !Selectable)
	{
		return;
	}

	// Handle potential selection conflicts
	if (SelectionData.GetSelectablesSelectors(Selectable).Num() > MaxSimultaneousSelections && SelectablesData.HasAnyConflict())
	{
		SELECTION_WARNING(TEXT("⚠️ Selectable %s conflict potentially resolved due to selector %s deselecting."), *Selectable->GetName(), *Selector->GetName());
		SelectablesData.ClearAllValidationFlags(Selectable);
	}

	// Handle deselection
	SelectionData.Remove(Selector);
	SelectablesData.SetPendingSelectionNotification(Selectable);

	FGameplayEventData Payload;
	Payload.Instigator = Selector;//GetOwner();
	Payload.Target = Selectable;
	Payload.EventTag = DeselectTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Selectable, Payload.EventTag, Payload);
}

void UValidationComponent::OnValidation_Implementation(FInstancedStruct Data)
{
	Server_CompleteValidation(Data);
}

AActor* UValidationComponent::PreProcessRegistrationEvent(FInstancedStruct RegistrationData)
{
	const FGameplayEventData* GameplayEventData = RegistrationData.GetPtr<FGameplayEventData>();
	AActor* Selectable = GameplayEventData ? const_cast<AActor*>(GameplayEventData->Target.Get()) : nullptr;
	if (!Selectable)
	{
		return nullptr;
	}
	SelectablesData.CleanUpInvalidSelectables();
	SelectionData.CleanUpInvalidSelections();

	return Selectable;
}

void UValidationComponent::PostProcessRegistrationEvent()
{
	if (SelectablesData.Items.Num() > 1)
	{
		SelectablesData.SortByLocation();
	}
}

void UValidationComponent::PostProcessSelectionEvent()
{
	const int32 TotalExpectedSelections = GetTotalExpectedSelections();
	//Assuming that selectionData's elements all contain a unique and valid selectors
	const bool bAllSelectionsMade = SelectionData.Items.Num() == TotalExpectedSelections;
	if (bAllSelectionsMade)
	{
		if (SelectablesData.HasCompleted(LockTag))
		{
			return;
		}
		int32 TotalUniqueSelections = 0;
		for (FSelectableItem& Item : SelectablesData.Items)
		{
			const int32 TotalSelectors = SelectionData.GetSelectablesSelectors(Item.Selectable).Num();

			const bool bHasUnresolvedConflict = TotalSelectors > MaxSimultaneousSelections && !Item.HasConflictFlag();
			if (bHasUnresolvedConflict)
			{
				SELECTION_WARNING(TEXT("⚠️ Selectable %s has conflict with %d selectors."), *Item.Selectable->GetName(), TotalSelectors);
				SelectablesData.SetWaiting(ConflictTag, Item.Selectable); //SelectablesData.SetWaitingConflict(Item.Selectable);
				continue;
			}

			const bool bHasUniqueSelection = TotalSelectors == MaxSimultaneousSelections;
			if (bHasUniqueSelection)
			{
				TotalUniqueSelections++;
				if (Item.HasConflictFlag())
				{
					SELECTION_WARNING(TEXT("✅ Selectable %s conflict resolved."), *Item.Selectable->GetName());
					SelectablesData.ClearAllValidationFlags(Item.Selectable);
				}
			}
			else if (TotalSelectors == 0 && Item.HasConflictFlag())
			{
				SelectablesData.ClearAllValidationFlags(Item.Selectable);
			}
		}

		const bool bHasUnresolvedLock = TotalUniqueSelections == TotalExpectedSelections && !SelectablesData.HasWaiting(LockTag) && !SelectablesData.HasPending(LockTag) && !SelectablesData.HasNotified(LockTag);
		if (bHasUnresolvedLock)
		{
			SELECTION_WARNING(TEXT("✅ All selections are unique. Setting waiting lock evaluation."));
			SelectablesData.SetWaiting(LockTag);
		}
		return;
	}

	if (SelectablesData.HasAnyLock())
	{
		SELECTION_WARNING(TEXT("⚠️ Selection count changed. Clearing lock evaluations."));
		SelectablesData.ClearAllValidationFlags();

		// A lock and conflict cannot be active at the same time, so the timer should only be active if a lock process is ongoing. But just in case, we check conflicts too.
		if (SelectablesData.HasAnyConflict())
		{
			SELECTION_ERROR(TEXT("SelectableData has conflict evaluations active while lock evaluations are cleared."));
			return;
		}

		if (SelectionEvaluation.IsActive(GetWorld()))
		{
			SELECTION_WARNING(TEXT("⏱️ Clearing selection evaluation timer due to lock evaluations being cleared."));
			SelectionEvaluation.ClearTimer(GetWorld());
		}
		else
		{
			SELECTION_WARNING(TEXT("🧠 No active selection evaluation timer to clear."));
			SelectionEvaluation.Reset();
		}
	}
}

bool UValidationComponent::ProcessedWaitingValidations(const FGameplayTag EvaluationTag)
{
	SELECTION_DISPLAY(TEXT("🧠IS WAITING FOR VALIDATION"));
	if (EvaluationTag == FGameplayTag::EmptyTag)
	{
		SelectablesData.ClearAllValidationFlags();
		SelectionEvaluation.ClearIfActive(GetWorld());
		return false;
	}
	SelectablesData.SetPending(EvaluationTag);
	return true;
}

bool UValidationComponent::ProcessedPendingValidations(const FGameplayTag EvaluationTag)
{
	SELECTION_WARNING(TEXT("🧠HAS PENDING EVALUATIONS"));

	if (SelectionEvaluation.IsActiveWithTag(GetWorld(), EvaluationTag))
	{
		SelectablesData.ClearPendingValidationNotification();

		SELECTION_WARNING(TEXT("⏱️ Selection evaluation timer already active for tag: %s."), *EvaluationTag.ToString());
		return true;
	}
	if (EvaluationTag != FGameplayTag::EmptyTag)
	{
		SelectablesData.SetNotified(EvaluationTag);
	}
	else
	{
		SelectablesData.ClearAllValidationFlags();
		SelectionEvaluation.ClearIfActive(GetWorld());
	}

	if (SelectablesData.HasAnyNotified())
	{
		if (ValidationDurationPerTag.Contains(EvaluationTag))
		{
			SelectionEvaluation.BaseDuration = ValidationDurationPerTag[EvaluationTag];
		}
		SelectionEvaluation.StartTimer(GetWorld(), FTimerDelegate::CreateUObject(this, &UValidationComponent::Server_ProcessValidationTimer), EvaluationTag);

		SELECTION_WARNING(TEXT("⏱️ Starting selection evaluation timer for tag: %s."), *EvaluationTag.ToString());
	}
	SELECTION_DISPLAY(TEXT("🧠Processed pending validations for tag: %s."), *EvaluationTag.ToString());
	return false;
}

void UValidationComponent::Server_ProcessValidationTransition_Implementation()
{
	const FGameplayTag EvaluationTag = GetValidationTag();

	if (SelectablesData.HasAnyWaiting())
	{
		SELECTION_DISPLAY( TEXT("🧠%hs IS WAITING FOR VALIDATION"), __FUNCTION__);
		if (ProcessedWaitingValidations(EvaluationTag))
		{
			return;
		}
	}
	else if (SelectablesData.HasAnyPending())
	{
		SELECTION_WARNING(TEXT("🧠%hs HAS PENDING VALIDATIONS"), __FUNCTION__);
		if (ProcessedPendingValidations(EvaluationTag))
		{
			return;
		}
	}

	if (SelectionData.HasPendingNotifications() || SelectablesData.HasPendingSelectionNotifications())
	{
		SELECTION_WARNING(TEXT("🛜There are pending selection notifications to be processed."));
		SelectionData.ClearPendingNotifications();
		SelectablesData.ClearPendingNotifications();

		if (EvaluationTag == FGameplayTag::EmptyTag)
		{
			SelectionEvaluation.ClearIfActive(GetWorld());
		}
		if (SelectionEvaluation.IsActive(GetWorld()) && !SelectionEvaluation.EvaluationTag.MatchesTag(EvaluationTag))
		{
			SELECTION_WARNING(TEXT("⏱️ Clearing existing selection evaluation timer due to new selection notifications."));
			SelectionEvaluation.ClearTimer(GetWorld());
		}

		FGameplayEventData Payload;
		Payload.Instigator = GetOwner();
		Payload.Target = GetOwner();
		Payload.EventTag = SelectionTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), Payload.EventTag, Payload);	//Handled in BP
	}
}

void UValidationComponent::Server_ProcessValidationTimer_Implementation()
{
	if (SelectablesData.HasAnyCompleted())
	{
		FGameplayEventData Payload;
		Payload.Instigator = GetOwner();
		Payload.Target = GetOwner();
		Payload.EventTag = SelectionEvaluation.EvaluationTag;

		SelectionEvaluation.ClearTimer(GetWorld());
		SELECTION_WARNING(TEXT("🧠HAS COMPLETED %s"), *Payload.EventTag.ToString());
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), Payload.EventTag, Payload);
	}

	if (SelectablesData.HasAnyNotified())
	{
		SelectionEvaluation.DecreaseRemainingDuration(SelectionEvaluation.Rate);

        SELECTION_WARNING(TEXT("⏱️ Selection evaluation timer elapsed. Evaluating selections now. %s %d"), *SelectionEvaluation.EvaluationTag.ToString(),
        					 SelectionEvaluation.GetRemainingDurationInSeconds());

		if (!SelectionEvaluation.IsComplete())
		{
			return;
		}
		SelectablesData.SetCompleted(SelectionEvaluation.EvaluationTag);
	}
}

void UValidationComponent::Server_CompleteValidation_Implementation(FInstancedStruct CompletionData)
{
	const FGameplayEventData* GameplayEventData = CompletionData.GetPtr<FGameplayEventData>();
	if (!GameplayEventData)
	{
		return;
	}

	SELECTION_DISPLAY(TEXT("%hs called with EventTag: %s"),__FUNCTION__, *GameplayEventData->EventTag.ToString());
	const FGameplayTag EventTag = GameplayEventData->EventTag;
	if (EventTag.MatchesTag(LockTag))
	{
		CompleteLockValidation();
		return;
	}
	if (EventTag.MatchesTag(ConflictTag))
	{
		CompleteConflictValidation();
	}
}

void UValidationComponent::CompleteLockValidation()
{
	SELECTION_WARNING(TEXT("✅ Processing completed lock evaluation."));
	for (const FSelectableItem& Item : SelectablesData.Items)
	{
		if (!SelectablesData.HasCompleted(LockTag, Item.Selectable))
		{
			SELECTION_ERROR(TEXT("%hs, Selectable %s does not have completed lock state when processing completed lock evaluation."), __FUNCTION__,*Item.Selectable->GetName());
			continue;
		}

		FGameplayEventData Payload;
		Payload.Instigator = GetOwner();
		Payload.Target = Cast<AActor>(SelectionData.GetSelector(Item.Selectable));
		Payload.EventTag = LockTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Cast<AActor>(Item.Selectable), Payload.EventTag, Payload);
		//UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), Payload.EventTag, Payload);

	}

	if (SelectablesData.HasCompleted(LockTag))
	{
		SetComponentTickEnabled(false);
	}
}

void UValidationComponent::CompleteConflictValidation()
{
	SELECTION_WARNING(TEXT("⚠️ Processing completed conflict evaluation."));
	for (const FSelectableItem& Item : SelectablesData.Items)
	{
		if (!SelectablesData.HasCompleted(ConflictTag, Item.Selectable))
		{
			continue;
		}
		TArray<UObject*> Selectors = SelectionData.GetSelectablesSelectors(Item.Selectable);
		if (Selectors.Num() < 2)
		{
			SELECTION_ERROR(TEXT("Selectable %s does not have multiple selectors when processing completed conflict evaluation."), *Item.Selectable->GetName());
			continue;
		}

		const int32 RandomIndex = FMath::RandRange(0, Selectors.Num() - 1);
		FGameplayEventData Payload;
		Payload.Instigator = Cast<AActor>(Selectors[RandomIndex]);
		Payload.Target = Cast<AActor>(Item.Selectable);
		Payload.EventTag = EVENT_TAG_INTERACT;

		SELECTION_WARNING(TEXT("✅ Selector %s chosen randomly to resolve conflict on selectable %s."), *Payload.Instigator->GetName(), *Payload.Target->GetName());

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), Payload.EventTag, Payload);

	}
}

FGameplayTag UValidationComponent::GetValidationTag()
{
	int32 TotalExpectedSelections = GetTotalExpectedSelections();
	int32 UniqueSelections = 0, ConflictingSelections = 0;

	for (const FSelectableItem& Item : SelectablesData.Items) //Assuming that selectionData's elements all contain a unique and valid selectors
	{
		if (const int32 TotalSelectors = SelectionData.GetSelectablesSelectors(Item.Selectable).Num();
			TotalSelectors > MaxSimultaneousSelections)
		{
			ConflictingSelections++;
		}
		else if (TotalSelectors == 1)
		{
			UniqueSelections++;
		}
	}


	if (UniqueSelections == TotalExpectedSelections && ConflictingSelections == 0)
	{
		return LockTag;
	}

	if (ConflictingSelections > 0)
	{
		return ConflictTag;
	}
	return FGameplayTag::EmptyTag;
}

int32 UValidationComponent::GetTotalExpectedSelections() const
{
	if (!bAllowDynamicExpectedSelectionCount || DynamicExpectedSelectionCount < 0)
	{
		return GetWorld()->GetGameState()->PlayerArray.Num();
	}
	return DynamicExpectedSelectionCount;
}
