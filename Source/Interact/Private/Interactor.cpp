// Furkan approves of this


#include "Interactor.h"
#include "Dev/InteractLog.h"


void IInteractor::OnSuperFinishedInteraction_Implementation(FInstancedStruct InteractionData)
{
	INTERACT_WARNING( TEXT("%hs is not implemented"), __FUNCTION__)
}

void IInteractor::OnFinishedInteraction_Implementation(const UObject* Interactable)
{
	INTERACT_WARNING( TEXT("%hs is not implemented"), __FUNCTION__)
}

bool IInteractor::IsInteracting_Implementation() const
{
	return false;
}

int32 IInteractor::GetOwnerID_Implementation() const
{
	return -1;
}

FName IInteractor::GetOwnerName_Implementation() const
{
	return NAME_None;
}
