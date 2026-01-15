// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Async/Async_WaitGameplayEvent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayUtilFunctionLibrary.generated.h"


UCLASS()
class GAMEPLAYUTILS_API UGameplayUtilFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Gameplay Utils|Events")
	static void SendGameplayEventToActor(AActor* PayloadInstigator, AActor* PayloadTarget, const FGameplayTag EventTag, AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category="Gameplay Utils|Events")
	static FGameplayEventData CreateGameplayEventData(AActor* PayloadInstigator, AActor* PayloadTarget, const FGameplayTag EventTag);

	UFUNCTION(BlueprintCallable, Category="Gameplay Utils|Events")
	static void ApplyGameplayEffectToActor(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

};
