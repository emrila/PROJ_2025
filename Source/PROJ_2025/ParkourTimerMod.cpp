// Furkan approves of this


#include "ParkourTimerMod.h"

#include "ParkourManager.h"
#include "ParkourTimeTrialGoal.h"
#include "RoomLoader.h"
#include "WizardGameInstance.h"
#include "WizardGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Player/Characters/PlayerCharacterBase.h"
#include "Player/Controllers/PlayerControllerBase.h"



void UParkourTimerMod::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UParkourTimerMod, PlayersThatMadeIt);
}

void UParkourTimerMod::OnRep_PlayersThatMadeIt() const
{
	TimerWidget->UpdatePlayersThatMadeIt(PlayersThatMadeIt.Num());
}

void UParkourTimerMod::OnRoomEntered(ARoomManagerBase* InRoomManager)
{
	Super::OnRoomEntered(InRoomManager);
	UE_LOG(LogTemp, Warning, TEXT("ALL CLIENTS READY SUB"));
	AActor* GoalActor = UGameplayStatics::GetActorOfClass(GetWorld(), AParkourTimeTrialGoal::StaticClass());
	if (AParkourTimeTrialGoal* Goal = Cast<AParkourTimeTrialGoal>(GoalActor))
	{
		Goal->GoalTrigger->OnComponentBeginOverlap.AddDynamic(this, &UParkourTimerMod::OnOverlapBegin);
		UE_LOG(LogTemp, Display, TEXT("finded and binded goal"));
	}else
	{
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Tryna find PKA"));
	AActor* PKA = UGameplayStatics::GetActorOfClass(GetWorld(), AParkourManager::StaticClass());
	if (PKA)
	{
		if (AParkourManager* PM = Cast<AParkourManager>(PKA))
		{
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle, 
				this, 
				&UParkourTimerMod::DealDamageToPlayers, 
				PM->TimerIfTimeTrial + 4.f, 
				false                  
			);
		}
	}else
	{
		UE_LOG(LogTemp, Display, TEXT("PKA MIA"));
	}
}


void UParkourTimerMod::OnAllClientsReady()
{
	
}

void UParkourTimerMod::OnExitsUnlocked()
{
	if (TimerWidget)
	{
		Multicast_RemoveTimerWidget();
	}
}

void UParkourTimerMod::Multicast_RemoveTimerWidget_Implementation()
{
	TimerWidget->RemoveTimer();
}

void UParkourTimerMod::BeginReplication()
{
	Super::BeginReplication();
	AActor* PKA = UGameplayStatics::GetActorOfClass(GetWorld(), AParkourManager::StaticClass());
	if (PKA)
	{
		if (AParkourManager* PM = Cast<AParkourManager>(PKA))
		{
			Multicast_AddTimerWidget(PM->TimerIfTimeTrial);
		}
	}
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		APlayerControllerBase* MyPC = Cast<APlayerControllerBase>(PC);
		if (MyPC)
		{
			UE_LOG(LogTemp, Warning, TEXT("Client calls beginRep"));
			MyPC->Server_RegisterModifierClient(this);
		}
	}
}

void UParkourTimerMod::DealDamageToPlayers()
{
	if (AWizardGameState* GS = Cast<AWizardGameState>(GetWorld()->GetGameState()))
	{
		GS->DamageHealth(100.f);
	}
}

void UParkourTimerMod::Multicast_AddTimerWidget(float Timer)
{
	TSubclassOf<UUserWidget> TimerWidgetClass;
	if (UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetWorld()->GetGameInstance()))
	{
		TimerWidgetClass = GI->RoomLoader->ParkourTimerWidget;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue; 

		if (!TimerWidget)
		{
			TimerWidget = CreateWidget<UTimerUserWidget>(PC, TimerWidgetClass);
			TimerWidget->Timer = Timer;
			UE_LOG(LogTemp, Error, TEXT("TIMER SET TO %f"), TimerWidget->Timer);
			TimerWidget->AddToViewport();
		}
	}
}

void UParkourTimerMod::Multicast_FinishTimer_Implementation()
{
	TimerWidget->FinishTimer();
}

void UParkourTimerMod::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	APlayerCharacterBase* Pawn = Cast<APlayerCharacterBase>(OtherActor);
	if (!Pawn) return;

	APlayerState* PS = Pawn->GetPlayerState();
	if (!PS) return;

	if (PlayersThatMadeIt.Contains(PS)) return;

	PlayersThatMadeIt.Add(PS);

	UE_LOG(LogTemp, Warning, TEXT("Player %s finished parkour! Total: %d"),
		*PS->GetPlayerName(), PlayersThatMadeIt.Num());

	OnRep_PlayersThatMadeIt();

	if (AWizardGameState* GS = Cast<AWizardGameState>(GetWorld()->GetGameState()))
	{
		if (PlayersThatMadeIt.Num() == GS->PlayerArray.Num())
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
			Multicast_FinishTimer();
		}
	}
}
