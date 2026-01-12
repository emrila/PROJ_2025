#include "Data/AttributeData.h"
#include "Util/UpgradeLog.h"

bool FAttributeBase::IsValidProperty(const FProperty* Property)
{
	return Property && (
		Property->IsA<FFloatProperty>() ||
		Property->IsA<FIntProperty>()
	);
}

bool FAttributeFloat::Modify(const FModiferData ModifierData)
{
	const bool bIsModified = AttemptModifyContainer<float, FFloatProperty>(ModifierData);
	if (bIsModified)
	{
		const float NewValue = GetValueFromContainer<float, FFloatProperty>();
		UPGRADE_DISPLAY(TEXT("%hs: Modified Float attribute to new value: %f"), __FUNCTION__, NewValue);		
	}	
	return bIsModified;
}

bool FAttributeInt32::Modify(const FModiferData ModifierData)
{
	const bool bIsModified = AttemptModifyContainer<int32, FIntProperty>(ModifierData);
	if (bIsModified)
	{
		const int32 NewValue = GetValueFromContainer<int32, FIntProperty>();
		UPGRADE_DISPLAY(TEXT("%hs: Modified Int32 attribute to new value: %d"), __FUNCTION__, NewValue);
	}
	return bIsModified;
}
