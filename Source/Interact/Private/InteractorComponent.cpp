// Furkan approves of this


#include "InteractorComponent.h"

#include "Dev/InteractLog.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

namespace InteractUtil
{
	bool Trace(AActor* Owner, const float InteractionRadius, const float InteractionDistance, FHitResult& Hit, const EDrawDebugTrace::Type DebugType = EDrawDebugTrace::None)
	{
		FVector Start;
		FRotator ViewRot;
		Owner->GetActorEyesViewPoint(Start, ViewRot);
		const FVector End = Start + ViewRot.Vector() * InteractionDistance;
		const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
		return UKismetSystemLibrary::SphereTraceSingle(Owner, Start, End, InteractionRadius, TraceChannel,false,{Owner}, DebugType,Hit,true);
	}
}

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f;

	InteractionRadius = 40.f;
	InteractionDistance = 1000.f;
	bInteracting = false;
}


void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();
	SetInteracting(false);
	SetTargetInteractable(nullptr);
}


void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TraceForInteractable();

}

void UInteractorComponent::TraceForInteractable()
{
	auto IsInteractable = [this] (UObject* Object) -> bool{
		return Object && Object->Implements<UInteractable>() && Execute_CanInteract(Object);
	};

	if (bInteracting)
	{
		INTERACT_WARNING( TEXT("Currently interacting, will not trace for interactable"));
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner) // Should not happen though
	{
		return;
	}
	FHitResult Hit;
	if (!InteractUtil::Trace(Owner,InteractionRadius, InteractionDistance, Hit))
	{
		//INTERACT_DISPLAY( TEXT("Nothing found in trace"));
		return;
	}

	if (IsInteractable(Hit.GetActor()))
	{
		SetTargetInteractable(Hit.GetActor());
		INTERACT_DISPLAY( TEXT("Found Actor interactable: %s"), *GetNameSafe(Hit.GetActor()));
	}
	else if (IsInteractable(Hit.GetComponent()))
	{
		SetTargetInteractable(Hit.GetComponent());
		INTERACT_DISPLAY( TEXT("Found Component interactable: %s"), *GetNameSafe(Hit.GetComponent()));
	}
}

void UInteractorComponent::ClearInteractable()
{
	INTERACT_DISPLAY(TEXT("Clearing interactable"));
	SetInteracting(false);
	SetTargetInteractable(nullptr);
}

void UInteractorComponent::OnInteract_Implementation(UObject* Interactor)
{
	if (!Execute_CanInteract(this))
	{
		INTERACT_WARNING( TEXT("Trying to interact, but cannot interact! IsInteracting: %s, TargetInteractable: %s"), bInteracting ? TEXT("true") : TEXT("false"), *GetNameSafe(TargetInteractable.GetObject()));
		return;
	}

	SetInteracting(true);

	if (!TargetInteractable.GetObject())
	{
		INTERACT_WARNING( TEXT("TargetInteractable is null when trying to interact!"));
		ClearInteractable();
		return;
	}

	Execute_OnInteract(TargetInteractable.GetObject(), this);
}

bool UInteractorComponent::CanInteract_Implementation()
{
	return !bInteracting && TargetInteractable.GetObject();
}

void UInteractorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInteractorComponent, bInteracting);
}

void UInteractorComponent::SetTargetInteractable(const TScriptInterface<IInteractable> InTargetInteractable)
{
	TargetInteractable = InTargetInteractable;
	INTERACT_DISPLAY( TEXT("Setting target interactable to: %s"), *GetNameSafe(TargetInteractable.GetObject()));
}

void UInteractorComponent::OnFinishedInteraction_Implementation(const UObject* Interactable)
{
	ClearInteractable();
}
