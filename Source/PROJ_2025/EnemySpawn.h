// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "EnemySpawn.generated.h"

UCLASS(Blueprintable)
class PROJ_2025_API AEnemySpawn : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawn();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawn")
	TSubclassOf<AActor> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Editor")
	USkeletalMeshComponent* PreviewMesh;

protected:
	// Editor-only visual components
#if WITH_EDITORONLY_DATA
	// Shows the enemy mesh


	// Always-visible box
	UPROPERTY(VisibleAnywhere, Category="Editor")
	UBoxComponent* SpawnBox;
#endif

public:
#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif
};