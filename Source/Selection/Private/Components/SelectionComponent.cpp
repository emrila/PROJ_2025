// 🐲Furkan approves of this🐲


#include "Components/SelectionComponent.h"

#include "Async/Async_WaitGameplayEvent.h"
#include "Utility/Tags.h"

USelectionComponent::USelectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SelectionTag = EVENT_TAG_SELECTION;
	SelectTag = EVENT_TAG_SELECT;
	DeselectTag = EVENT_TAG_DESELECT;
	ValidationTag = EVENT_TAG_VALIDATION;
	LockTag = EVENT_TAG_VALIDATION_LOCK;
	ConflictTag = EVENT_TAG_VALIDATION_CONFLICT;
	ConflictResolvedTag = EVENT_TAG_VALIDATION_CONFLICT_RESOLUTION;
	RegistrationTag = EVENT_TAG_REGISTRATION_REGISTER;
	UnregistrationTag = EVENT_TAG_REGISTRATION_UNREGISTER;
}

void USelectionComponent::BeginPlay()
{
	Super::BeginPlay();

	WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(GetOwner(), ValidationTag, false, false))->EventReceived.AddDynamic(
		this, &USelectionComponent::Server_OnValidation);
}

void USelectionComponent::Server_OnValidation_Implementation(FGameplayEventData Payload)
{
	Execute_OnValidation(this, FInstancedStruct::Make<FGameplayEventData>(Payload));
}

void USelectionComponent::Server_OnRequestSelection_Implementation(FGameplayEventData Payload)
{
	Execute_OnRequestSelection(this, FInstancedStruct::Make<FGameplayEventData>(Payload));
}

void USelectionComponent::Server_OnRegisterSelectable_Implementation(FGameplayEventData Payload)
{
	Execute_OnRegisterSelectable(this, FInstancedStruct::Make<FGameplayEventData>(Payload));
}

void USelectionComponent::Server_OnUnregisterSelectable_Implementation(FGameplayEventData Payload)
{
	Execute_OnUnregisterSelectable(this, FInstancedStruct::Make<FGameplayEventData>(Payload));
}
