// 🐲Furkan approves of this🐲


#include "Components/CustomAbilitySystemComponent.h"

#include "Utility/GameplayUtilFunctionLibrary.h"


UCustomAbilitySystemComponent::UCustomAbilitySystemComponent()
{
}

TArray<FGameplayAbilitySpecHandle> UCustomAbilitySystemComponent::GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant)
{
	TArray<FGameplayAbilitySpecHandle> AbilityHandles;

	if (GetOwner()->HasAuthority())
	{
		for (const TSubclassOf Ability : AbilitiesToGrant)
		{
			FGameplayAbilitySpecHandle SpecHandle = GiveAbility(FGameplayAbilitySpec(Ability, 1, -1, GetOwner()));
			AbilityHandles.Add(SpecHandle);
		}
		SendAbilitiesChangedEvent();
	}
	return AbilityHandles;
}

void UCustomAbilitySystemComponent::SendAbilitiesChangedEvent()
{
	static FGameplayTag AbilitiesChangedTag = FGameplayTag::RequestGameplayTag("Event.Abilities.Changed");
	UGameplayUtilFunctionLibrary::SendGameplayEventToActor( GetOwner(), GetOwner(), AbilitiesChangedTag, GetOwner());
}

void UCustomAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	bool bAbilitiesChanged = false;
	if (LastActivatedAbility.Num() != ActivatableAbilities.Items.Num())
	{
		bAbilitiesChanged = true;
	}
	else
	{
		for (int32 i = 0; i < LastActivatedAbility.Num(); i++)
		{
			if (LastActivatedAbility[i].Ability != ActivatableAbilities.Items[i].Ability)
			{
				bAbilitiesChanged = true;
				break;
			}
		}
	}

	if (bAbilitiesChanged)
	{
		SendAbilitiesChangedEvent();
		LastActivatedAbility = ActivatableAbilities.Items;
	}
}
