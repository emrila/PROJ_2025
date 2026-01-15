#include "UpgradeUtil.h"
#include "Data/AttributeData.h"

FProperty* UpgradeUtils::GetProperty(const UObject* Owner, const FName PropertyName)
{
	if (!Owner)
	{
		return nullptr;
	}

	return Owner->GetClass()->FindPropertyByName(PropertyName);
}

bool UpgradeUtils::IsValidProperty(const FProperty* Property)
{
	return Property && (
		Property->IsA<FFloatProperty>() ||
		Property->IsA<FIntProperty>()
	);
}

TUniquePtr<FAttributeBase> UpgradeUtils::CreateAttribute(UObject* InOwner, FProperty* InProperty, const FName InRowName)
{
	if (!InProperty)
	{
		return nullptr;
	}

	if (InProperty->IsA<FFloatProperty>())
	{
		return MakeUnique<FAttributeFloat>(InOwner, InProperty, InRowName);
	}
	if (InProperty->IsA<FIntProperty>())
	{
		return MakeUnique<FAttributeInt32>(InOwner, InProperty, InRowName);
	}

	return nullptr;
}
