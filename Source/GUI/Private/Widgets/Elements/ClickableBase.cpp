// 🐲Furkan approves of this🐲


#include "Widgets/Elements/ClickableBase.h"

void UClickableBase::CallOnClicked()
{
	OnClicked.Broadcast();
}

void UClickableBase::CallOnPressed()
{
	OnPressed.Broadcast();
}

void UClickableBase::CallOnReleased()
{
	OnReleased.Broadcast();
}

void UClickableBase::CallOnHovered()
{
	OnHovered.Broadcast();
}

void UClickableBase::CallOnUnhovered()
{
	OnUnhovered.Broadcast();
}

void UClickableBase::CallOnFocused()
{
	OnFocused.Broadcast();
}

void UClickableBase::CallOnUnfocused()
{
	OnUnfocused.Broadcast();
}
