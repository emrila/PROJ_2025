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

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="Spawn")
	int WaveNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Editor")
	USkeletalMeshComponent* PreviewMesh;

protected:

#if WITH_EDITORONLY_DATA



	UPROPERTY(VisibleAnywhere, Category="Editor")
	UBoxComponent* SpawnBox;
#endif


};