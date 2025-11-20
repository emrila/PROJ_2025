// Furkan approves of this


#include "Util/UpgradeFunctionLibrary.h"

#include "Core/UpgradeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "World/UpgradeSpawner.h"

AUpgradeSpawner* UUpgradeFunctionLibrary::GetLocalUpgradeSpawner(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	AActor* ActorOfClass = UGameplayStatics::GetActorOfClass(WorldContextObject, AUpgradeSpawner::StaticClass());	
	return ActorOfClass ? Cast<AUpgradeSpawner>(ActorOfClass) : nullptr;		
}

UUpgradeComponent* UUpgradeFunctionLibrary::GetLocalUpgradeComponent(UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;	
	 if (!World)
	 {
		 return nullptr;
	 }
	UUpgradeComponent* UpgradeComponent = nullptr;
	for(FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		const APlayerController* PlayerController = Iterator->Get();
		if (!PlayerController)
		{
			continue;
		}
		if (!PlayerController || !PlayerController->IsLocalController())
		{
			continue;
		}

		const APawn* Pawn = PlayerController->GetPawnOrSpectator();
		UpgradeComponent = Pawn ? Pawn->FindComponentByClass<UUpgradeComponent>() : nullptr;
		break;
	}

	 return UpgradeComponent;
}
