// 🐲Furkan approves of this🐲

#include "Components/SelectableComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Async/Async_WaitGameplayEvent.h"
#include "Utility/SelectionLog.h"

void USelectableComponent::BeginPlay()
{
	Super::BeginPlay();

	WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(GetOwner(), SelectionTag, false, false))->
	EventReceived.AddDynamic(this, &USelectableComponent::Server_OnRequestSelection);

	if (bAutoRegister && Execute_CanRegister(this))
	{
		Server_OnRegisterSelectable(FGameplayEventData());
	}
}

void USelectableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bAutoRegister && Execute_CanRegister(this))
	{
		Server_OnUnregisterSelectable(FGameplayEventData());
	}
	Super::EndPlay(EndPlayReason);
}

void USelectableComponent::OnRegisterSelectable_Implementation(FInstancedStruct RegistrationData)
{
	FGameplayEventData Payload;
	Payload.Instigator = GetOwner();
	Payload.Target = GetOwner();
	Payload.EventTag = RegistrationTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetTargetOwner(), Payload.EventTag, Payload);
}

void USelectableComponent::OnUnregisterSelectable_Implementation(FInstancedStruct UnregistrationData)
{
	FGameplayEventData Payload ;
	Payload.Instigator = GetOwner();
	Payload.Target = GetOwner();
	Payload.EventTag = UnregistrationTag;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetTargetOwner(), Payload.EventTag, Payload);
}

bool USelectableComponent::CanRegister_Implementation() const
{
	const AActor* TargetOwner = GetTargetOwner();
	return TargetOwner && TargetOwner->HasAuthority();
}

void USelectableComponent::OnRequestSelection_Implementation(FInstancedStruct RequestData)
{
	const FGameplayEventData* Payload = RequestData.GetPtr<FGameplayEventData>();
	if (Payload->EventTag.MatchesTag(SelectTag))
	{
		SELECTION_DISPLAY(TEXT("🛜 Selectable %s received selection request."), *GetOwner()->GetName());
		Execute_OnSelected(GetOwner(), RequestData);
		return;
	}
	if (Payload->EventTag.MatchesTag(DeselectTag))
	{
		SELECTION_DISPLAY(TEXT("🛜 Selectable %s received deselection request."), *GetOwner()->GetName());
		Execute_OnDeselected(GetOwner(), RequestData);
	}
}

void USelectableComponent::OnValidation_Implementation(const FInstancedStruct ValidationData)
{
	Execute_OnValidation(GetOwner(), ValidationData);
}

AActor* USelectableComponent::GetTargetOwner() const
{
	return bUseOwnersOwnerAsTarget && GetOwner()
		       ? GetOwner()->GetOwner()
		       : GetOwner();
}
