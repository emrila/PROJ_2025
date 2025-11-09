// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomExit.h"
#include "GameFramework/Actor.h"
#include "RoomManagerBase.generated.h"

UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Combat      UMETA(DisplayName = "Combat"),
	Parkour    UMETA(DisplayName = "Parkour"),
	Camp        UMETA(DisplayName = "Camp"),
	Event       UMETA(DisplayName = "Event"),
	Boss        UMETA(DisplayName = "Boss"),
};

UCLASS(BlueprintType)
class URoomData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> RoomLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ERoomType RoomType;
};

UCLASS()
class PROJ_2025_API ARoomManagerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomManagerBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

};
