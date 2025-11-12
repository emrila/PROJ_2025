
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dagger.generated.h"

UCLASS()
class PROJ_2025_API ADagger : public AActor
{
	GENERATED_BODY()

public:
	ADagger();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* Mesh;
	
};
