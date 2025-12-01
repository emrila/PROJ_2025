// Furkan approves of this


#include "ParkourTimerMod.h"

#include "ParkourManager.h"
#include "ParkourTimeTrialGoal.h"
#include "RoomLoader.h"
#include "WizardGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Player/Characters/PlayerCharacterBase.h"

void UParkourTimerMod::OnRoomEntered(ARoomManagerBase* InRoomManager)
{
	Super::OnRoomEntered(InRoomManager);

	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this, FName("SetupAfterRoomEntered"), InRoomManager);
    
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 0.5f, false);
	}
}
void UParkourTimerMod::SetupAfterRoomEntered(ARoomManagerBase* InRoomManager)
{
	if (!InRoomManager) return;

	AActor* GoalActor = UGameplayStatics::GetActorOfClass(GetWorld(), AParkourTimeTrialGoal::StaticClass());
	if (AParkourTimeTrialGoal* Goal = Cast<AParkourTimeTrialGoal>(GoalActor))
	{
		Goal->GoalTrigger->OnComponentBeginOverlap.AddDynamic(this, &UParkourTimerMod::OnOverlapBegin);
		Multicast_AddTimerWidget(); // RPC to spawn widgets for all clients
	}
}

void UParkourTimerMod::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UParkourTimerMod, PlayersThatMadeIt);
}

void UParkourTimerMod::OnRep_PlayersThatMadeIt() const
{
	TimerWidget->UpdatePlayersThatMadeIt(PlayersThatMadeIt.Num());
}

void UParkourTimerMod::BeginReplication()
{
	Super::BeginReplication();
	Multicast_AddTimerWidget();
}

void UParkourTimerMod::Multicast_AddTimerWidget()
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast_AddTimerWidget called! World role: %s, NetMode: %d"), 
		*UEnum::GetValueAsString(GetOwner()->GetLocalRole()), 
		GetWorld()->GetNetMode());
	TSubclassOf<UUserWidget> TimerWidgetClass;
	if (UWizardGameInstance* GI = Cast<UWizardGameInstance>(GetWorld()->GetGameInstance()))
	{
		TimerWidgetClass = GI->RoomLoader->ParkourTimerWidget;
	}
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue; // Only the local client spawns the widget

		if (!TimerWidget)
		{
			TimerWidget = CreateWidget<UTimerUserWidget>(PC, TimerWidgetClass);
			TimerWidget->AddToViewport();
		}
	}
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
}
