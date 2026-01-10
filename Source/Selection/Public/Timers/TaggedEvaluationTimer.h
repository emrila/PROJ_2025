// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "TaggedEvaluationTimer.generated.h"

USTRUCT(BlueprintType)
struct FTaggedEvaluationTimer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDuration = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float RemainingDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Rate = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FGameplayTag EvaluationTag = FGameplayTag::EmptyTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FTimerHandle EvaluationTimerHandle;

	FTaggedEvaluationTimer();

	void DecreaseRemainingDuration(float InRate);

	int32 GetRemainingDurationInSeconds() const;

	void StartTimer(const UWorld* World, const FTimerDelegate& TimerDelegate, FGameplayTag InTag = FGameplayTag::EmptyTag, const bool
	bIgnoreExisting = false);

	void ClearTimer(const UWorld* World, FGameplayTag InTag = FGameplayTag::EmptyTag);

	void ClearIfActiveWithTag(const UWorld* World, FGameplayTag InTag);

	void ClearIfActive(const UWorld* World, FGameplayTag InTag = FGameplayTag::EmptyTag);

	bool IsActive(const UWorld* World) const;

	bool IsComplete() const;

	void Reset(FGameplayTag InTag = FGameplayTag::EmptyTag);

	bool IsActiveWithTag(const UWorld* World, FGameplayTag InTag) const;
};
