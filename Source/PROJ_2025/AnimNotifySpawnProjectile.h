#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotifySpawnProjectile.generated.h"

UCLASS(meta=(DisplayName = "Spawn Projectile"))
class PROJ_2025_API UAnimNotifySpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName SocketName;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,  const FAnimNotifyEventReference& EventReference) override;
};