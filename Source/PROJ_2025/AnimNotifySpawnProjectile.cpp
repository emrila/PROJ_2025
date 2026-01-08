
#include "AnimNotifySpawnProjectile.h"

#include "CactusCharacter.h"
#include "GameFramework/Actor.h"

void UAnimNotifySpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,  const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner) return;

	UWorld* World = Owner->GetWorld();
	if (!World) return;
	
	if (!Owner->HasAuthority()) return;
	
	FVector SpawnLocation = MeshComp->GetComponentLocation();
	
	if (!SocketName.IsNone() && MeshComp->DoesSocketExist(SocketName))
	{
		SpawnLocation = MeshComp->GetSocketLocation(SocketName);
	}
	
	ACactusCharacter *Cactus = Cast<ACactusCharacter>(Owner); 
	if (!Cactus) return;
	
	Cactus->OnAttackAnimNotify.Broadcast(SpawnLocation);
}