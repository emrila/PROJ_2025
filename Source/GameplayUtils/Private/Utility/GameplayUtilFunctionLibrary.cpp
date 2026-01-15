// 🐲Furkan approves of this🐲


#include "Utility/GameplayUtilFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"

void UGameplayUtilFunctionLibrary::SendGameplayEventToActor(AActor* PayloadInstigator, AActor* PayloadTarget, const FGameplayTag EventTag, AActor* TargetActor)
{
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = PayloadInstigator;
	Payload.Target = PayloadTarget;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, Payload.EventTag, Payload);
}

FGameplayEventData UGameplayUtilFunctionLibrary::CreateGameplayEventData(AActor* PayloadInstigator, AActor* PayloadTarget, const FGameplayTag EventTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.Instigator = PayloadInstigator;
	Payload.Target = PayloadTarget;

	return Payload;
}

void UGameplayUtilFunctionLibrary::ApplyGameplayEffectToActor(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!ASC)
	{
		return;
	}
	ASC->ApplyGameplayEffectToSelf(GameplayEffectClass->GetDefaultObject<UGameplayEffect>(), 1.0f, ASC->MakeEffectContext());
}
