// 🐲Furkan approves of this🐲


#include "Utility/GameplayUtilFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"

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
