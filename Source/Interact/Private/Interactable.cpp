// Furkan approves of this


#include "Interactable.h"

#include "Dev/InteractLog.h"


void IInteractable::OnPreInteract_Implementation()
{
	INTERACT_WARNING( TEXT("%hs is not implemented"), __FUNCTION__)
}

void IInteractable::OnInteract_Implementation(UObject* Interactor)
{
	INTERACT_WARNING( TEXT("%hs is not implemented"), __FUNCTION__)
}

void IInteractable::OnPostInteract_Implementation()
{
	INTERACT_WARNING( TEXT("%hs is not implemented"), __FUNCTION__)
}

bool IInteractable::CanInteract_Implementation()
{
	return true;
}
