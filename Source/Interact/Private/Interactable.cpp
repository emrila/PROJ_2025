// Furkan approves of this


#include "Interactable.h"

#include "Dev/InteractLog.h"


void IInteractable::OnPreInteract_Implementation(UObject* Interactor)
{
	INTERACT_WARNING( TEXT("OnPreInteract is not implemented"))
}

void IInteractable::OnInteract_Implementation(UObject* Interactor)
{
	INTERACT_WARNING( TEXT("OnInteract is not implemented"))
}

void IInteractable::OnPostInteract_Implementation(UObject* Interactor)
{
	INTERACT_WARNING( TEXT("OnPostInteract is not implemented"))
}

void IInteractable::SetLocalPlayerController_Implementation(APlayerController* PlayerController)
{
	INTERACT_WARNING( TEXT("SetLocalPlayerController is not implemented"))
}

bool IInteractable::CanInteract_Implementation()
{
	return true;
}
