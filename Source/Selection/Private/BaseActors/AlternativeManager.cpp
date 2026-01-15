// 🐲Furkan approves of this🐲

#include "BaseActors/AlternativeManager.h"

#include "AbilitySystemComponent.h"
#include "Components/CustomAbilitySystemComponent.h"
#include "Components/ValidationComponent.h"
#include "GameplayUtils/Public/Async/Async_WaitGameplayEvent.h"
#include "GameplayUtils/Public/Utility/Tags.h"
#include "Net/UnrealNetwork.h"

AAlternativeManager::AAlternativeManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
	PrimaryActorTick.bStartWithTickEnabled = false;

	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UCustomAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	ValidationComponent = CreateDefaultSubobject<UValidationComponent>(TEXT("ValidationComponent"));
	ValidationComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* AAlternativeManager::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAlternativeManager::BeginPlay()
{
	Super::BeginPlay();

	WaitGameplayEvents.Add_GetRef(UAsync_WaitGameplayEvent::ActivateAndWaitGameplayEventToActor(this, EVENT_TAG_INTERACT, false, false))->
	EventReceived.AddDynamic(this, &AAlternativeManager::WrapperOnInteractionEvent);
}

void AAlternativeManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAlternativeManager, AbilitySystemComponent);
	DOREPLIFETIME(AAlternativeManager, ValidationComponent);
}

void AAlternativeManager::WrapperOnInteractionEvent(FGameplayEventData Payload)
{
	Execute_OnRequestSelection(ValidationComponent, FInstancedStruct::Make<FGameplayEventData>(Payload));
}

FInstancedStruct AAlternativeManager::OnGetSelectablesInfo_Implementation(const UObject* Selectable)
{
	if (ValidationComponent)
	{
		return FInstancedStruct::Make<FSelectablesInfos>();
	}
	return Selectable
	? FInstancedStruct::Make<FSelectablesInfo>(ValidationComponent->GetSelectableInfo(Selectable))
	: FInstancedStruct::Make<FSelectablesInfos>(ValidationComponent->GetAllSelectablesInfo());

}
