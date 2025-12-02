// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "RoomManagerBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RoomInstanceLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PROJ_2025_API URoomInstanceLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category="Room")
	static FRoomInstance MakeRoomInstance(URoomData* RoomData, TArray<TSubclassOf<URoomModifierBase>> ActiveModifierClasses)
	{
		FRoomInstance Instance;
		Instance.RoomData = RoomData;
		Instance.ActiveModifierClasses = ActiveModifierClasses;
		return Instance;
	}
};
