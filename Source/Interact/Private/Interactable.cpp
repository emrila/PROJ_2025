// Furkan approves of this


#include "Interactable.h"

#include "Dev/InteractLog.h"


void IInteractable::OnInteract_Implementation(UObject* Interactor)
{
	INTERACT_WARNING( TEXT("OnInteract is not implemented"))
}

bool IInteractable::CanInteract_Implementation()
{
	return true;
}
