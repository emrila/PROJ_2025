// 🐲Furkan approves of this🐲


#include "Components/ValidationComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Async/Async_WaitGameplayEvent.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/SelectionDisplayInterface.h"
#include "Net/UnrealNetwork.h"
#include "Utility/GameplayUtilFunctionLibrary.h"
#include "Utility/SelectionLog.h"
#include "Utility/Tags.h"

UValidationComponent::UValidationComponent()
{
	ValidationDataMap.Add(LockTag, 2.0f);
	ValidationDataMap.Add(ConflictTag, 5.0f);

	SelectionEffectMap.Add(SelectionTag, nullptr);
	SelectionEffectMap.Add(DeselectTag, nullptr);
}

TArray<FSelectablesInfo> UValidationComponent::GetAllSelectablesInfo() const
{
	TArray<FSelectablesInfo> SelectablesInfo;
	for (const auto& Item : SelectablesData.Items)
	{
		SelectablesInfo.Emplace(
			Item.Selectable,
			SelectionData.GetSelectablesSelectors(Item.Selectable),
			Item.EvaluationFlags,
			Item.HasConflictFlag() ? ConflictTag : Item.HasCompletedLock() ? LockTag : FGameplayTag::EmptyTag);
	}

	return SelectablesInfo;
}

FSelectablesInfo UValidationComponent::GetSelectableInfo(const UObject* Selectable) const
{
	for (const auto& Item : SelectablesData.Items)
	{
		if (Item.Selectable == Selectable)
		{
			return FSelectablesInfo(
				Item.Selectable,
				SelectionData.GetSelectablesSelectors(Item.Selectable),
				Item.EvaluationFlags,
				Item.HasConflictFlag() ? ConflictTag : Item.HasCompletedLock() ? LockTag : FGameplayTag::EmptyTag);
		}
	}
	return FSelectablesInfo();
}

TArray<FSelectablesInfo> UValidationComponent::GetSelectableInfoForSelector(const UObject* Selector) const
{
	TArray<FSelectablesInfo> SelectablesInfo;
	for (const UObject* Selectable : SelectionData.GetSelectorsSelectables(Selector))
	{
		for (const auto& Item : SelectablesData.Items)
		{
			if (Item.Selectable != Selectable)
			{
				continue;
			}
			SelectablesInfo.Emplace(
				Item.Selectable,
				SelectionData.GetSelectablesSelectors(Item.Selectable),
				Item.EvaluationFlags,
				Item.HasConflictFlag() ? ConflictTag : Item.HasCompletedLock() ? LockTag : FGameplayTag::EmptyTag);
			break;
		}
	}

	return SelectablesInfo;
}

void UValidationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(GetOwner(), RegistrationTag, false, false))->EventReceived.AddDynamic(
			this, &UValidationComponent::Server_OnRegisterSelectable);

		WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(GetOwner(), UnregistrationTag, false, false))->EventReceived.AddDynamic(
			this, &UValidationComponent::Server_OnUnregisterSelectable);

		
		GetOwner()->GetWorldTimerManager().SetTimer(ValidationTransitionTimerHandle, FTimerDelegate::CreateUObject(this, &UValidationComponent::Server_ProcessValidationTransition), 1.0f, true, 2.0f);
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
	AActor* Selectable = PreProcessSelectableFromRegistrationEvent(RegistrationData);
	if (!Selectable)
	{
		return;
	}

	SELECTION_DISPLAY(TEXT("👨‍👩‍👧‍👦 Registering: %s with: %s as Selectable at location %s."), *Selectable->GetName(), *GetName(), *Selectable->GetActorLocation().ToString());
	SelectablesData.AddOrUpdate(Selectable, Selectable->GetActorLocation());
	//	PostProcessRegistrationEvent();
}

void UValidationComponent::OnUnregisterSelectable_Implementation(FInstancedStruct UnregistrationData)
{
	const AActor* Selectable = PreProcessSelectableFromRegistrationEvent(UnregistrationData);
	if (!Selectable)
	{
		return;
	}
	SELECTION_DISPLAY(TEXT("👨‍👩‍👧‍👦 Unregistering: %s with: %s as Selectable at location %s."), *Selectable->GetName(), *GetName(), *Selectable->GetActorLocation().ToString());
	SelectablesData.Remove(Selectable);
	//	PostProcessRegistrationEvent();
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
		SELECTION_DISPLAY(TEXT("%hs Selector %s attempted to select already selected Selectable %s. Deselecting now."), __FUNCTION__, *Selector->GetName(), *Selectable->GetName());
		Execute_OnDeselected(this, RequestData);
	}
	else
	{
		SELECTION_DISPLAY(TEXT("%hs Selector %s selecting Selectable %s."), __FUNCTION__, *Selector->GetName(), *Selectable->GetName());
		// Assuming single selection per selector, and new selection should be prioritized -> Deselect previous selection if found
		if (UObject* CurrentSelectable = SelectionData.GetSelectable(Selector))
		{
			if (!bAllowDeselection || bMustDeselectBeforeNewSelection)
			{
				return;
			}

			SELECTION_DISPLAY(TEXT("%hs Selector %s had previous Selectable %s. Deselecting now."), __FUNCTION__, *Selector->GetName(), *CurrentSelectable->GetName());
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
	if (!GetOwner() || !GetOwner()->HasAuthority() || !GameplayEventData)
	{
		return;
	}
	AActor* Selectable = const_cast<AActor*>(GameplayEventData->Target.Get());
	AActor* Selector = const_cast<AActor*>(GameplayEventData->Instigator.Get());
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

	OnApplySelectionEffectToSelector(SelectionTag, Selector);
	SendEventToSelectable(Selector, Selectable, SelectTag, Selectable);
}

void UValidationComponent::OnDeselected_Implementation(FInstancedStruct Data)
{
	const FGameplayEventData* GameplayEventData = Data.GetPtr<FGameplayEventData>();
	if (!GetOwner() || !GetOwner()->HasAuthority() || !GameplayEventData || !bAllowDeselection)
	{
		return;
	}

	AActor* Selectable = const_cast<AActor*>(GameplayEventData->Target.Get());
	AActor* Selector = const_cast<AActor*>(GameplayEventData->Instigator.Get());
	if (!Selector || !Selectable)
	{
		return;
	}

	// Handle potential selection conflicts
	if (SelectionData.GetSelectablesSelectors(Selectable).Num() > MaxSimultaneousSelections && SelectablesData.HasAnyConflict())
	{
		SELECTION_DISPLAY(TEXT("⚠️ Selectable %s conflict potentially resolved due to selector %s deselecting."), *Selectable->GetName(), *Selector->GetName());
		OnClearAllValidationFlags(Selectable);
	}

	// Handle deselection
	SelectionData.Remove(Selector);
	SelectablesData.SetPendingSelectionNotification(Selectable);

	SendEventToSelectable(Selector, Selectable, DeselectTag, Selectable);
	OnApplySelectionEffectToSelector(DeselectTag, Selector);
}

void UValidationComponent::OnValidation_Implementation(const FInstancedStruct Data)
{
	Server_CompleteValidation(Data);
}

AActor* UValidationComponent::PreProcessSelectableFromRegistrationEvent(const FInstancedStruct& RegistrationData)
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
				SelectablesData.SetWaiting(ConflictTag, Item.Selectable);

				AActor* Selectable = Cast<AActor>(Item.Selectable);
				SendEventToSelectable(GetOwner(), Selectable, ConflictTag, Selectable);
				continue;
			}

			if (TotalSelectors == MaxSimultaneousSelections)
			{
				TotalUniqueSelections++;
				if (Item.HasConflictFlag())
				{
					SELECTION_WARNING(TEXT("✅ Selectable %s conflict resolved."), *Item.Selectable->GetName());
					OnClearAllValidationFlags(Item.Selectable);
				}
			}
			else if (TotalSelectors == 0 && Item.HasConflictFlag())
			{
				OnClearAllValidationFlags(Item.Selectable);
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
		OnClearAllValidationFlags();

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
		OnClearAllValidationFlags();
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
		OnClearAllValidationFlags();
		SelectionEvaluation.ClearIfActive(GetWorld());
	}

	if (SelectablesData.HasAnyNotified())
	{
		if (ValidationDataMap.Contains(EvaluationTag))
		{
			SelectionEvaluation.BaseDuration = ValidationDataMap[EvaluationTag].Duration;
		}

		SelectionEvaluation.StartTimer(GetWorld(), FTimerDelegate::CreateUObject(this, &UValidationComponent::Server_ProcessValidationTimer), EvaluationTag);

		if (EvaluationTag.MatchesTag(ConflictTag))
		{
			for (UObject* Object : SelectionData.GetSelectablesWithMultipleSelectors(MaxSimultaneousSelections))
			{
				OnApplyValidationEffect(ConflictTag, Object);
			}

			SendEventToOwner(GetOwner(), GetOwner(), ConflictTag);
		}

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
		SELECTION_DISPLAY(TEXT("🧠%hs IS WAITING FOR VALIDATION"), __FUNCTION__);
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

		OnSetSelectionInfo();
		SendEventToOwner(GetOwner(), GetOwner(), SelectionTag); // Handled in BP
	}
}

void UValidationComponent::Server_ProcessValidationTimer_Implementation()
{
	const FGameplayTag EventTag = SelectionEvaluation.EvaluationTag;
	if (SelectablesData.HasAnyCompleted())
	{
		SelectionEvaluation.ClearTimer(GetWorld());
		SELECTION_WARNING(TEXT("🧠HAS COMPLETED %s"), *EventTag.ToString());

		SendEventToOwner(GetOwner(), GetOwner(), EventTag);
	}

	if (SelectablesData.HasAnyNotified())
	{
		SelectionEvaluation.DecreaseRemainingDuration(SelectionEvaluation.Rate);
		SELECTION_WARNING(TEXT("⏱️ Selection evaluation timer elapsed. Evaluating selections now. %s %d"), *EventTag.ToString(), SelectionEvaluation.GetRemainingDurationInSeconds());
		if (!SelectionEvaluation.IsComplete())
		{
			return;
		}
		SelectablesData.SetCompleted(EventTag);
	}
}

void UValidationComponent::Server_CompleteValidation_Implementation(FInstancedStruct CompletionData)
{
	const FGameplayEventData* GameplayEventData = CompletionData.GetPtr<FGameplayEventData>();
	if (!GameplayEventData)
	{
		return;
	}

	SELECTION_DISPLAY(TEXT("%hs called with EventTag: %s"), __FUNCTION__, *GameplayEventData->EventTag.ToString());
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

	FSelectablesInfos SelectablesInfoArray;
	for (const FSelectableItem& Item : SelectablesData.Items)
	{
		if (!SelectablesData.HasCompleted(LockTag, Item.Selectable))
		{
			SELECTION_ERROR(TEXT("%hs, Selectable %s does not have completed lock state when processing completed lock evaluation."), __FUNCTION__, *Item.Selectable->GetName());
			continue;
		}

		AActor* Selector = Cast<AActor>(SelectionData.GetSelector(Item.Selectable));
		AActor* Selectable = Cast<AActor>(Item.Selectable);

		OnApplyValidationEffect(LockTag, Item.Selectable);
		SendEventToSelectable(Selectable, Selector, LockTag, Selectable);

		SelectablesInfoArray.Items.Add(GetSelectableInfo(Item.Selectable));

	}

	Execute_OnValidation(GetOwner(), FInstancedStruct::Make<FSelectablesInfos>(SelectablesInfoArray));

	if (SelectablesData.HasCompleted(LockTag))
	{
		SetComponentTickEnabled(false);
	}

	FValidationData* ValidationData = ValidationDataMap.Find(LockTag);
	if (!ValidationData || !ValidationData->EffectClass)
	{
		return;
	}
	UGameplayUtilFunctionLibrary::ApplyGameplayEffectToActor(Cast<AActor>(GetOwner()), ValidationData->EffectClass);
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
		if (Selectors.Num() <= MaxSimultaneousSelections)
		{
			continue;
		}

		const int32 WinnerIndex = FMath::RandRange(0, Selectors.Num() - 1);
		AActor* Selectable = Cast<AActor>(Item.Selectable);

		for (int32 i = 0; i < Selectors.Num(); i++)
		{
			if (WinnerIndex == i)
			{
				continue;
			}

			UObject* Selector = Selectors[i];

			SELECTION_WARNING(TEXT("✅ Selector %s chosen randomly to resolve conflict on selectable %s."), *Cast<AActor>(Selector)->GetName(), *Selectable->GetName());
			SendEventToOwner(Cast<AActor>(Selector), Selectable, EVENT_TAG_INTERACT);
		}

		OnApplyValidationEffect(ConflictResolvedTag, Item.Selectable);
		SendEventToSelectable(GetOwner(), Cast<AActor>(Selectors[WinnerIndex]), ConflictResolvedTag, Selectable);
	}
}

FGameplayTag UValidationComponent::GetValidationTag()
{
	const int32 TotalExpectedSelections = GetTotalExpectedSelections();
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
	if ((!bAllowDynamicExpectedSelectionCount || DynamicExpectedSelectionCount < 0)
		&& GetWorld() && GetWorld()->GetGameState())
	{
		return GetWorld()->GetGameState()->PlayerArray.Num();
	}
	return DynamicExpectedSelectionCount;
}

void UValidationComponent::OnClearAllValidationFlags(UObject* Selectable)
{
	SELECTION_DISPLAY(TEXT("%hs: 🧹Clearing all validation flags."), __FUNCTION__);
	SelectablesData.ClearAllValidationFlags(Selectable);
	OnApplySelectionEffectToSelectable(ValidationTag, Selectable);
}

void UValidationComponent::OnSetSelectionInfo(UObject* Selectable) const
{
	if (Selectable && Selectable->Implements<USelectionDisplayInterface>())
	{
		const FInstancedStruct SelectablesInfo = FInstancedStruct::Make<FSelectablesInfo>(GetSelectableInfo(Selectable));
		ISelectionDisplayInterface::Execute_OnSetSelectablesInfo(Selectable, SelectablesInfo);
		ISelectionDisplayInterface::Execute_OnProcessSelectablesInfo(Selectable);
	}
}

void UValidationComponent::OnApplyValidationEffect(const FGameplayTag Tag, UObject* Selectable)
{
	FValidationData* ValidationData = ValidationDataMap.Find(Tag);
	if (!ValidationData || !ValidationData->EffectClass)
	{
		SELECTION_ERROR(TEXT("%hs: No validation data found for tag %s."), __FUNCTION__, *Tag.ToString());
		return;
	}

	if (Selectable)
	{
		OnSetSelectionInfo(Selectable);
		UGameplayUtilFunctionLibrary::ApplyGameplayEffectToActor(Cast<AActor>(Selectable), ValidationData->EffectClass);
	}
	else
	{
		for (const FSelectableItem& Item : SelectablesData.Items)
		{
			OnSetSelectionInfo(Item.Selectable);
			UGameplayUtilFunctionLibrary::ApplyGameplayEffectToActor(Cast<AActor>(Item.Selectable), ValidationData->EffectClass);
		}
	}

	SELECTION_DISPLAY(TEXT("%hs: Applied validation effect for tag %s."), __FUNCTION__, *Tag.ToString());
}

void UValidationComponent::OnApplySelectionEffectToSelector(const FGameplayTag Tag, UObject* Selector)
{
	const TSubclassOf<UGameplayEffect>* EffectClassPtr = SelectionEffectMap.Find(Tag);
	const TSubclassOf<UGameplayEffect> EffectClass = EffectClassPtr && *EffectClassPtr ? *EffectClassPtr : nullptr;

	if (!EffectClass)
	{
		SELECTION_ERROR(TEXT("%hs: No selection effect found for tag %s."), __FUNCTION__, *Tag.ToString());
		return;
	}

	if (Selector)
	{
		UGameplayUtilFunctionLibrary::ApplyGameplayEffectToActor(Cast<AActor>(Selector), EffectClass);
	}
	else
	{
		for (const auto& Item : SelectionData.Items)
		{
			UGameplayUtilFunctionLibrary::ApplyGameplayEffectToActor(Cast<AActor>(Item.Selector), EffectClass);
		}
	}

	SELECTION_DISPLAY(TEXT("%hs: Applied selection effect for tag %s."), __FUNCTION__, *Tag.ToString());

}

void UValidationComponent::OnApplySelectionEffectToSelectable(const FGameplayTag Tag, UObject* Selectable)
{
	const TSubclassOf<UGameplayEffect>* EffectClassPtr = SelectionEffectMap.Find(Tag);
	const TSubclassOf<UGameplayEffect> EffectClass = EffectClassPtr && *EffectClassPtr ? *EffectClassPtr : nullptr;

	if (!EffectClass)
	{
		SELECTION_ERROR(TEXT("%hs: No selection effect found for tag %s."), __FUNCTION__, *Tag.ToString());
		return;
	}

	if (Selectable)
	{
		UGameplayUtilFunctionLibrary::ApplyGameplayEffectToActor(Cast<AActor>(Selectable), EffectClass);
	}
	else
	{
		for (const auto& Item : SelectablesData.Items)
		{
			UGameplayUtilFunctionLibrary::ApplyGameplayEffectToActor(Cast<AActor>(Item.Selectable), EffectClass);
		}
	}
	SELECTION_DISPLAY(TEXT("%hs: Applied selection effect %s for tag %s."), __FUNCTION__, *EffectClass->GetName(), *Tag.ToString());
}

void UValidationComponent::SendEventToOwner(AActor* PayloadInstigator, AActor* PayloadTarget, const FGameplayTag EventTag) const
{
	UGameplayUtilFunctionLibrary::SendGameplayEventToActor(PayloadInstigator, PayloadTarget, EventTag, GetOwner());
}

void UValidationComponent::SendEventToSelectable(AActor* PayloadInstigator, AActor* PayloadTarget, const FGameplayTag EventTag, AActor* Selectable) const
{
	OnSetSelectionInfo(Selectable);
	UGameplayUtilFunctionLibrary::SendGameplayEventToActor(PayloadInstigator, PayloadTarget, EventTag, Selectable);
}
