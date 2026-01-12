// 🐲Furkan approves of this🐲


#include "Components/InteractableComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Async/Async_WaitGameplayEvent.h"
#include "Dev/InteractLog.h"
#include "Utility/Tags.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	InteractTag = EVENT_TAG_INTERACT;
	ProximityTag = EVENT_TAG_PROXIMITY;
	EnterTag = EVENT_TAG_PROXIMITY_ENTER;
	ExitTag = EVENT_TAG_PROXIMITY_EXIT;
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	GetOwner()->OnActorBeginOverlap.AddDynamic(this, &UInteractableComponent::OnActorBeginOverlap);
	GetOwner()->OnActorEndOverlap.AddDynamic(this, &UInteractableComponent::OnActorEndOverlap);

	WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(GetOwner(), ProximityTag, false, false))->EventReceived.AddDynamic(
	this, &UInteractableComponent::OnProximityEvent);
}

void UInteractableComponent::OnActorBeginOverlap([[maybe_unused]] AActor* OverlappedActor, AActor* OtherActor)
{
	OnOverlap(OtherActor, EnterTag);
}

void UInteractableComponent::OnActorEndOverlap([[maybe_unused]] AActor* OverlappedActor, AActor* OtherActor)
{
	OnOverlap(OtherActor, ExitTag);
}

void UInteractableComponent::OnComponentBeginOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComp, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex, [[maybe_unused]] bool bFromSweep, [[maybe_unused]] const FHitResult& SweepResult)
{
	OnOverlap(OtherActor, EnterTag);
}

void UInteractableComponent::OnComponentEndOverlap([[maybe_unused]] UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, [[maybe_unused]] UPrimitiveComponent* OtherComp, [[maybe_unused]] int32 OtherBodyIndex)
{
	OnOverlap(OtherActor, ExitTag);
}

void UInteractableComponent::OnProximityEvent(FGameplayEventData Payload)
{
	if (Payload.EventTag.MatchesTag(EnterTag))
	{
		Execute_OnEnterInteractableRange(GetOwner(), FInstancedStruct::Make<FGameplayEventData>(Payload));
	}
	else if (Payload.EventTag.MatchesTag(ExitTag))
	{
		Execute_OnExitInteractableRange(GetOwner(), FInstancedStruct::Make<FGameplayEventData>(Payload));
	}
}

void UInteractableComponent::OnOverlap(AActor* OtherActor, const FGameplayTag EventTag) const
{
	if (!OtherActor)
	{
		return;
	}
	if (const APawn* OverlappingPawn = Cast<APawn>(OtherActor);
		OverlappingPawn && OverlappingPawn->IsLocallyControlled())
	{
		FGameplayEventData Payload;
		Payload.Instigator = OtherActor;
		Payload.Target = GetOwner();
		Payload.EventTag = EventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), Payload.EventTag, Payload);
	}
}

AActor* UInteractableComponent::GetTargetOwner() const
{
	return bUseOwnersOwnerAsTarget && GetOwner() ? GetOwner()->GetOwner() : GetOwner();
}

void UInteractableComponent::Server_OnInteract_Implementation(UObject* Interactor)
{
	INTERACT_HI_FROM(__FUNCTION__);
	if (bInteractionInputEnabled) // See declaration for "SetInteractionInputEnabled" for explanation
	{
		SetInteractionInputEnabled(false);
		FTimerHandle TimerHandle;
		const FTimerDelegate TimerDelegate = FTimerDelegate::CreateUObject(this, &UInteractableComponent::SetInteractionInputEnabled, true);
		GetOwner()->GetWorldTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.1f, false);
	}
	else
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.Instigator = nullptr;
	Payload.Target = GetOwner();
	Payload.EventTag = InteractTag;

	if (AActor* PayloadInstigator = Cast<AActor>(Interactor))
	{
		Payload.Instigator = PayloadInstigator;
	}
	else if (const UActorComponent* PayloadInstigatorOwner = Cast<UActorComponent>(Interactor))
	{
		Payload.Instigator = PayloadInstigatorOwner->GetOwner();
	}

	if (Payload.Instigator)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetTargetOwner(), Payload.EventTag, Payload);
	}
}
