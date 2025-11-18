// Furkan approves of this


#include "Interactor.h"
#include "Dev/InteractLog.h"


void IInteractor::OnFinishedInteraction_Implementation(const UObject* Interactable)
{
	INTERACT_WARNING( TEXT("OnFinishedInteraction is not implemented"))
}

bool IInteractor::IsInteracting_Implementation() const
{
	return false;
}

int32 IInteractor::GetOwnerID_Implementation() const
{
	return -1;
}
