// 🐲Furkan approves of this🐲


#include "Data/TeamModifierData.h"

void FTeamModifierData::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	if (Modifiers.IsEmpty())
	{
		const FModifierEntry Item{InRowName};
		Modifiers.Add(Item);
	}
}
