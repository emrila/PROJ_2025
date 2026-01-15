// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AnimAssetCompleteTask.generated.h"

UCLASS()
class GAMEPLAYUTILS_API UAnimAssetCompleteTask : public UAbilityTask
{
	GENERATED_BODY()
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnimAssetCompleteTaskComplete);

	UPROPERTY(BlueprintAssignable)
	FOnAnimAssetCompleteTaskComplete OnComplete;

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = true))
	static UAnimAssetCompleteTask* WaitForAnimationAsset(UGameplayAbility* OwningAbility, UAnimationAsset* AnimationAsset);

	virtual void Activate() override;
	virtual void TickTask(float DeltaTime) override;

protected:
	UPROPERTY()
	UAnimationAsset* AnimAsset;

	bool bIsTicking = false;
	float ElapsedTime = 0.f;
	float AnimDuration = 0.f;

	void EndAnimationTask();
};
