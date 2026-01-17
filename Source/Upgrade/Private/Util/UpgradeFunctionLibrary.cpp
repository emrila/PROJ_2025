// Furkan approves of this


#include "Util/UpgradeFunctionLibrary.h"
#include "Components/UpgradeComponent.h"
#include "Interfaces/UpgradeSystemInterface.h"
#include "Kismet/GameplayStatics.h"
#include "World/UpgradeSpawner.h"

AUpgradeSpawner* UUpgradeFunctionLibrary::GetUpgradeSpawner(UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (!World)
	{
		return nullptr;
	}

	AActor* FoundSpawner = UGameplayStatics::GetActorOfClass(World, AUpgradeSpawner::StaticClass());

	return FoundSpawner ? Cast<AUpgradeSpawner>(FoundSpawner) : nullptr;
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

UUpgradeComponent* UUpgradeFunctionLibrary::GetUpgradeComponentFromActor(const AActor* Actor)
{
	return Actor->Implements<UUpgradeSystemInterface>() ? Cast<IUpgradeSystemInterface>(Actor)->GetUpgradeComponent() : nullptr;
}
