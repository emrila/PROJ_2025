// 🐲Furkan approves of this🐲

#include "BaseActors/Alternative.h"
#include "AbilitySystemComponent.h"
#include "Components/InteractableComponent.h"
#include "Components/SelectableComponent.h"
#include "Interfaces/Interactor.h"
#include "Net/UnrealNetwork.h"

AAlternative::AAlternative()
{
	PrimaryActorTick.bCanEverTick = false;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	bReplicates = true;

	SelectableComponent = CreateDefaultSubobject<USelectableComponent>(TEXT("SelectableComponent"));
	SelectableComponent->SetIsReplicated(true);

	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));
	InteractableComponent->SetIsReplicated(true);
}

void AAlternative::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAlternative, AbilitySystemComponent);
	DOREPLIFETIME(AAlternative, SelectableComponent);
	DOREPLIFETIME(AAlternative, InteractableComponent);
	DOREPLIFETIME(AAlternative, SelectablesInfo);
}

UAbilitySystemComponent* AAlternative::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAlternative::OnInteract_Implementation(UObject* Interactor)
{
	InteractableComponent->Server_OnInteract(Interactor);
	IInteractor::Execute_OnFinishedInteraction(Interactor, this);
}

void AAlternative::OnSetSelectablesInfo_Implementation(FInstancedStruct Data)
{
	if (const FSelectablesInfo* Info = Data.GetPtr<FSelectablesInfo>())
	{
		SelectablesInfo = *Info;
	}

	OnRep_SelectablesInfo();
}

void AAlternative::OnClearSelectablesInfo_Implementation()
{
	SelectablesInfo = FSelectablesInfo();
}

FInstancedStruct AAlternative::OnGetSelectablesInfo_Implementation(const UObject* Selectable)
{
	if (!SelectablesInfo.Selectable && SelectableComponent->GetTargetOwner())
	{
		Execute_OnSetSelectablesInfo(this, Execute_OnGetSelectablesInfo(SelectableComponent->GetTargetOwner(), Selectable));
	}
	return FInstancedStruct::Make<FSelectablesInfo>(SelectablesInfo);
}

void AAlternative::OnRep_SelectablesInfo()
{
	Execute_OnProcessSelectablesInfo(this);
}
