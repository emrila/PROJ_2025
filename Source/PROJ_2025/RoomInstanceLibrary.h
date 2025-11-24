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
	static FRoomInstance MakeRoomInstance(URoomData* RoomData)
	{
		FRoomInstance Instance;
		Instance.RoomData = RoomData;
		Instance.ActiveModifiers = {};
		return Instance;
	}

	UFUNCTION(BlueprintCallable, Category="Room")
	static void AddModifierToRoom(FRoomInstance& RoomInstance, URoomModifierBase* Modifier)
	{
		RoomInstance.ActiveModifiers.Add(Modifier);
	}
};
