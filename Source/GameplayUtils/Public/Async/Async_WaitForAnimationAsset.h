// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Async_WaitForAnimationAsset.generated.h"


UCLASS()
class GAMEPLAYUTILS_API UAsync_WaitForAnimationAsset : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAsyncWaitForAnimationAsset);

	UPROPERTY(BlueprintAssignable)
	FOnAsyncWaitForAnimationAsset OnComplete;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UAsync_WaitForAnimationAsset* WaitForAnimationAsset(USkeletalMeshComponent* MeshComp, UAnimationAsset* AnimationAsset, bool bReverse = false);

	virtual void Activate() override;
	virtual void Tick(float DeltaTime);

protected:
	void EndAnimationTask();

private:
	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY()
	UAnimationAsset* AnimAsset;

	bool bIsTicking = false;
	bool bReverse = false;
	float ElapsedTime = 0.f;
	float AnimDuration = 0.f;
};
