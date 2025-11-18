// Furkan approves of this


#include "InteractorComponent.h"

#include "Dev/InteractLog.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

namespace InteractUtil
{
	bool Trace(AActor* Owner, const float InteractionRadius, const float InteractionDistance, FHitResult& Hit, const EDrawDebugTrace::Type DebugType = EDrawDebugTrace::None)
	{
		auto Sphere = [&]()
		{
			FVector Start;
			FRotator ViewRot;
			Owner->GetActorEyesViewPoint(Start, ViewRot);
			const FVector End = Start + ViewRot.Vector() * InteractionDistance;
			const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2); // ECC_GameTraceChannel2 = Interactable		
			return UKismetSystemLibrary::SphereTraceSingle(Owner, Start, End, InteractionRadius, TraceChannel,false,{Owner}, DebugType,Hit,true);
		};
		auto Capsule = [&]()
		{
			FVector Start;
			FRotator ViewRot;
			Owner->GetActorEyesViewPoint(Start, ViewRot);
			const FVector End = Start + ViewRot.Vector() * InteractionDistance;
			const auto HalfHeight = Owner->GetActorScale3D().Z/2.f;
			const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2); // ECC_GameTraceChannel2 = Interactable
			return UKismetSystemLibrary::CapsuleTraceSingle(Owner, Start, End, HalfHeight, InteractionRadius, TraceChannel,false,{Owner}, DebugType,Hit,true);
		};
		
		return Sphere();
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

	Server_SetInteracting(false);
	SetTargetInteractable(nullptr);
	if (const AController* InstController = GetOwner()->GetInstigatorController())	{
		const TObjectPtr<APawn> Pawn = InstController->GetPawn();
		SetComponentTickEnabled(Pawn->IsLocallyControlled());
		INTERACT_DISPLAY( TEXT("InteractorComponent tick enabled: %s"), Pawn->IsLocallyControlled() ? TEXT("true") : TEXT("false"));
	}
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

	if (!InteractUtil::Trace(Owner, InteractionRadius, InteractionDistance, Hit
	#if WITH_EDITORONLY_DATA
		, DebugType
	#endif
	))
	{
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
	Server_SetInteracting(false);
	SetTargetInteractable(nullptr);
}

void UInteractorComponent::OnInteract_Implementation(UObject* Interactor)
{
	if (!Execute_CanInteract(this))
	{
		INTERACT_WARNING( TEXT("Trying to interact, but cannot interact! IsInteracting: %s, TargetInteractable: %s"), bInteracting ? TEXT("true") : TEXT("false"), *GetNameSafe(TargetInteractable.GetObject()));
		return;
	}

	Server_SetInteracting(true);

	UObject* Interactable = TargetInteractable.GetObject();
	if (!Interactable)
	{
		INTERACT_WARNING( TEXT("TargetInteractable is null when trying to interact!"));
		ClearInteractable();
		return;
	}
	TargetInteractable->Execute_OnPreInteract(Interactable, this);
	Server_InteractWith(Interactable);
	TargetInteractable->Execute_OnPostInteract(Interactable);
}

bool UInteractorComponent::CanInteract_Implementation()
{
	return !bInteracting && TargetInteractable.GetObject();
}

void UInteractorComponent::Server_InteractWith_Implementation(UObject* Interactable)
{
	if (Interactable && Interactable->Implements<UInteractable>())
	{
		Execute_OnInteract(Interactable, this);
	}
}

void UInteractorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInteractorComponent, bInteracting);
}

void UInteractorComponent::Server_SetInteracting_Implementation(const bool bInInteracting)
{
	bInteracting = bInInteracting;
}

void UInteractorComponent::SetTargetInteractable(const TScriptInterface<IInteractable> InTargetInteractable)
{
	TargetInteractable = InTargetInteractable;
	INTERACT_DISPLAY( TEXT("Setting target interactable to: %s"), *GetNameSafe(TargetInteractable.GetObject()));
}

int32 UInteractorComponent::GetOwnerID_Implementation() const
{	
	return OwnerID;
}

void UInteractorComponent::OnSuperFinishedInteraction_Implementation(FInstancedStruct InteractionData)
{
	OnFinishedInteraction.Broadcast( InteractionData);
}

void UInteractorComponent::OnFinishedInteraction_Implementation(const UObject* Interactable)
{
	INTERACT_DISPLAY( TEXT("Finished interaction interactable"));
	ClearInteractable();
}

void UInteractorComponent::Server_SetOwnerID_Implementation(const int32 InOwnerID)
{
	OwnerID = InOwnerID;
	INTERACT_DISPLAY( TEXT("Setting owner id to %d"), OwnerID);
}
