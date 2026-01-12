// Furkan approves of this


#include "Components/InteractorComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Dev/InteractLog.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Utility/Tags.h"

namespace InteractUtil
{
	TTuple<FVector, FVector, ETraceTypeQuery> GetTraceStartEnd(const AActor* Owner, const float InteractionDistance)
	{
		FVector Start = Owner->GetActorLocation();
		FRotator ViewRot;
		Owner->GetActorEyesViewPoint(Start, ViewRot);
		const FVector End = Start + ViewRot.Vector() * InteractionDistance;
		const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2); // ECC_GameTraceChannel2 = Interactable

		return MakeTuple(Start, End, TraceChannel);
	}

	bool SphereTrace(AActor* Owner, const float InteractionRadius, const float InteractionDistance, FHitResult& Hit, const EDrawDebugTrace::Type DebugType = EDrawDebugTrace::None)
	{
		const auto [Start, End, TraceChannel] = GetTraceStartEnd(Owner, InteractionDistance);

		return UKismetSystemLibrary::SphereTraceSingle(Owner, Start, End, InteractionRadius, TraceChannel, false, {Owner}, DebugType, Hit, true);
	}

	bool CapsuleTrace(AActor* Owner, const float InteractionRadius, const float InteractionDistance, FHitResult& Hit, const EDrawDebugTrace::Type DebugType = EDrawDebugTrace::None)
	{
		const auto [Start, End, TraceChannel] = GetTraceStartEnd(Owner, InteractionDistance);

		constexpr float Half = 2.f;
		const auto HalfHeight = Owner->GetActorScale3D().Z / Half;

		return UKismetSystemLibrary::CapsuleTraceSingle(Owner, Start, End, InteractionRadius, HalfHeight, TraceChannel, false, {Owner}, DebugType, Hit, true);
	}
}

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	InteractTag = EVENT_TAG_INTERACT;
	ProximityTag = EVENT_TAG_PROXIMITY;
	EnterTag = EVENT_TAG_PROXIMITY_ENTER;
	ExitTag = EVENT_TAG_PROXIMITY_EXIT;
}

void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority())
	{
		GetOwner()->OnActorBeginOverlap.AddDynamic(this, &UInteractorComponent::OnActorBeginOverlap);
		GetOwner()->OnActorEndOverlap.AddDynamic(this, &UInteractorComponent::OnActorEndOverlap);
		Server_SetInteracting(false);
		Server_SetTargetInteractable(nullptr);
		SetComponentTickEnabled(true);
	}
}

void UInteractorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInteractorComponent, bInteracting);
	DOREPLIFETIME(UInteractorComponent, TargetInteractableObject);
}

void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TraceForInteractable();
}

void UInteractorComponent::TraceForInteractable()
{
	if (bInteracting)
	{
		INTERACT_WARNING(TEXT("Currently interacting, will not trace for interactable"));
		return;
	}

	FHitResult Hit;
	const bool bHit = InteractUtil::SphereTrace(
		GetOwner(),
		InteractionRadius,
		InteractionDistance,
		Hit
#if WITH_EDITORONLY_DATA
		, DebugType
#endif
	);

	if (bHit)
	{
		UObject* TargetObject = nullptr;
		if (Hit.GetActor() && Hit.GetActor()->Implements<UInteractable>() && Execute_CanInteract(Hit.GetActor()))
		{
			TargetObject = Hit.GetActor();
		}
		else if (Hit.GetComponent() && Hit.GetComponent()->Implements<UInteractable>() && Execute_CanInteract(Hit.GetComponent()))
		{
			TargetObject = Hit.GetComponent();
		}

		if (!TargetObject)
		{
			return;
		}

		if (TargetInteractableObject && TargetInteractableObject != TargetObject)
		{
			OnSetTargetInteractable(nullptr, TargetInteractableObject, ExitTag);
		}

		OnSetTargetInteractable(TargetObject, TargetObject, EnterTag);
	}
	else if (TargetInteractableObject)
	{
		OnSetTargetInteractable(nullptr, TargetInteractableObject, ExitTag);
	}
}

void UInteractorComponent::OnSetTargetInteractable(const UObject* InTargetInteractable, const UObject* EventTarget, const FGameplayTag EventTag)
{
	if (GetOwner()->HasAuthority())
	{
		Server_SetTargetInteractable(InTargetInteractable);
		Client_SendProximityEvent(EventTarget, EventTag);
	}
}

void UInteractorComponent::OnActorBeginOverlap([[maybe_unused]] AActor* OverlappedActor, AActor* OtherActor)
{
	if (bInteracting
		|| !OtherActor
		|| OtherActor == GetOwner()
		|| !OtherActor->Implements<UInteractable>()
		|| !Execute_CanInteract(OtherActor))
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		if (TargetInteractableObject)
		{
			OnSetTargetInteractable(nullptr, TargetInteractableObject, ExitTag);
		}
		OnSetTargetInteractable(OtherActor, OtherActor, EnterTag);
	}

}

void UInteractorComponent::OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (bInteracting
		|| !OtherActor
		|| OtherActor == GetOwner()
		|| !OtherActor->Implements<UInteractable>()
		|| !Execute_CanInteract(OtherActor))
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		if (TargetInteractableObject)
		{
			OnSetTargetInteractable(nullptr, TargetInteractableObject, ExitTag);
		}
		OnSetTargetInteractable( OtherActor, OtherActor, ExitTag);
	}
}

void UInteractorComponent::ClearInteractable()
{
	INTERACT_DISPLAY(TEXT("Clearing interactable"));
	Server_SetInteracting(false);
	if (GetOwner()->HasAuthority())
	{
		INTERACT_DISPLAY(TEXT("%hs: Authority - clearing interactable on clients"), __FUNCTION__);
		Server_SetTargetInteractable(nullptr);
	}
	else
	{
		INTERACT_DISPLAY(TEXT("%hs: Non-Authority - clearing interactable locally"), __FUNCTION__);
		SetTargetInteractable(nullptr);
	}
}

void UInteractorComponent::OnInteract_Implementation(UObject* Interactor)
{
	if (!Execute_CanInteract(this))
	{
		INTERACT_WARNING(TEXT("Trying to interact, but cannot interact! IsInteracting: %s, TargetInteractable: %s"), bInteracting ? TEXT("true") : TEXT("false"),
		                 *GetNameSafe(TargetInteractableObject));
		return;
	}

	if (!TargetInteractableObject || !TargetInteractableObject->Implements<UInteractable>())
	{
		INTERACT_WARNING(TEXT("TargetInteractable is null when trying to interact!"));
		ClearInteractable();
		return;
	}

	Server_SetInteracting(true);
	Execute_OnInteract(TargetInteractableObject, this);
}

bool UInteractorComponent::CanInteract_Implementation()
{
	return !bInteracting && TargetInteractableObject;
}

void UInteractorComponent::SetInteracting(const bool bInInteracting)
{
	bInteracting = bInInteracting;
}

void UInteractorComponent::SetTargetInteractable(UObject* InTargetInteractable)
{
	if (InTargetInteractable && TargetInteractableObject == InTargetInteractable)
	{
		return;
	}

	TargetInteractableObject = InTargetInteractable;
	INTERACT_DISPLAY(TEXT("Setting target interactable to: %s"), *GetNameSafe(TargetInteractableObject));
}

void UInteractorComponent::Server_SetTargetInteractable_Implementation(const UObject* InTargetInteractable)
{
	SetTargetInteractable(const_cast<UObject*>(InTargetInteractable));
}

void UInteractorComponent::SendProximityEvent(const UObject* InTargetInteractable, const FGameplayTag EventTag)
{
	AActor* TargetActor = Cast<AActor>(const_cast<UObject*>(InTargetInteractable));
	if (!TargetActor)
	{
		if (const UActorComponent* TargetComp = Cast<UActorComponent>(InTargetInteractable))
		{
			TargetActor = TargetComp->GetOwner();
		}
	}

	if (TargetActor)
	{
		FGameplayEventData Payload;
		Payload.Instigator = GetOwner();
		Payload.Target = TargetActor;
		Payload.EventTag = EventTag;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, Payload.EventTag, Payload);
	}
}

void UInteractorComponent::Client_SendProximityEvent_Implementation(const UObject* InTargetInteractable, const FGameplayTag EventTag)
{
	SendProximityEvent(InTargetInteractable, EventTag);
}

void UInteractorComponent::Server_SetInteracting_Implementation(const bool bInInteracting)
{
	SetInteracting(bInInteracting);
}

void UInteractorComponent::OnFinishedInteraction_Implementation(const UObject* Interactable)
{
	INTERACT_HI_FROM(__FUNCTION__);
	ClearInteractable();
}

void UInteractorComponent::OnSendInteractionData_Implementation(FInstancedStruct Data)
{
	INTERACT_HI_FROM(__FUNCTION__);
	OnFinishedInteraction.Broadcast(Data);
}


void UInteractorComponent::Server_OnInteract_Implementation()
{
	Execute_OnInteract(this, TargetInteractableObject);
}
