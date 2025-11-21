// Furkan approves of this


#include "RoomLoader.h"

#include "WizardGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ARoomLoader::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARoomLoader, CurrentRoom);
	DOREPLIFETIME(ARoomLoader, PendingNextRoomData);
	DOREPLIFETIME(ARoomLoader, PastSevenRooms);
}
void ARoomLoader::Multicast_AddProgressWidget_Implementation()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->IsLocalController()) return;

	if (ProgressWidgetClass)
	{
		UUserWidget* Widget = CreateWidget(PC, ProgressWidgetClass);
		if (Widget)
		{
			Widget->AddToViewport();
		}
	}
}

void ARoomLoader::IncrementProgress(const bool CountAsClearedRoom)
{
	if (CountAsClearedRoom)
	{
		ClearedRooms++;
	}
	OneTwoThreeScale++;
	if (OneTwoThreeScale >= 3)
	{
		OneTwoThreeScale = 0;
		DungeonScaling += IncrementPerScale;
	}
	
}

float ARoomLoader::GetDungeonScaling() const
{
	return DungeonScaling;
}

void ARoomLoader::RegisterNextRoom(URoomData* RoomData)
{
	if (PastSevenRooms.Num() > 6)
	{
		PastSevenRooms.Empty();
	}
	PastSevenRooms.Add(RoomData);
	Multicast_AddProgressWidget();
}

TArray<URoomData*> ARoomLoader::GetPreviousRooms()
{
	return PastSevenRooms;
}


void ARoomLoader::LoadNextRoom_Implementation(URoomData* NextRoomData)
{
	PendingNextRoomData = NextRoomData;
	UWorld* World = GetWorld();
	if (!World || !PendingNextRoomData) return;
	if (!CurrentLoadedLevelName.IsNone())
	{
		FLatentActionInfo UnloadInfo;
		UnloadInfo.CallbackTarget = this;
		UnloadInfo.ExecutionFunction = FName("OnPreviousLevelUnloaded"); 
		UnloadInfo.Linkage = 0;
		UnloadInfo.UUID = __LINE__;
		
		UGameplayStatics::UnloadStreamLevel(this, CurrentLoadedLevelName, UnloadInfo, true);
		return;
	}
	OnPreviousLevelUnloaded();
}

void ARoomLoader::BeginPlay()
{
	Super::BeginPlay();
	if (UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetGameInstance()))
	{
		GI->RoomLoader = this;
	}
}

void ARoomLoader::OnNextLevelLoaded()
{
	AActor* RoomManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ARoomManagerBase::StaticClass());
	if (ARoomManagerBase* RoomManager = Cast<ARoomManagerBase>(RoomManagerActor))
	{
		RoomManager->OnRoomInitialized();
	}
}


void ARoomLoader::OnPreviousLevelUnloaded()
{
	if (!PendingNextRoomData)
	{
		UE_LOG(LogTemp, Warning, TEXT("PendingNextRoomData is null, skipping level load."));
		return;
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Loading level: %s"),
	   *PendingNextRoomData->RoomLevel.ToString()));
	
	FString LevelName = PendingNextRoomData->RoomLevel.GetAssetName();

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = FName("OnNextLevelLoaded");
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__;
	
	UGameplayStatics::LoadStreamLevel(
		this,                  
		FName(LevelName),             
		true,                 
		true,                  
		LatentInfo 
	);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Success")));
	CurrentLoadedLevelName = FName(LevelName);
	CurrentRoom = PendingNextRoomData;
	PendingNextRoomData = nullptr;
	

}
