#include "Core/AttributeData.h"
#include "Dev/UpgradeLog.h"

bool FAttributeBase::IsValidProperty(const FProperty* Property)
{
	return Property && (
		Property->IsA<FFloatProperty>() ||
		Property->IsA<FIntProperty>()
	);
}

void FAttributeFloat::Modify(const FModiferData ModifierData)
{
	if (AttemptModifyContainer<float, FFloatProperty>(ModifierData))
	{
		const float NewValue = GetValueFromContainer<float, FFloatProperty>();
		UPGRADE_DISPLAY(TEXT("%hs: Modified Float attribute to new value: %f"), __FUNCTION__, NewValue);
	}
}

void FAttributeInt32::Modify(const FModiferData ModifierData)
{
	if (AttemptModifyContainer<int32, FIntProperty>(ModifierData))
	{
		const int32 NewValue = GetValueFromContainer<int32, FIntProperty>();
		UPGRADE_DISPLAY(TEXT("%hs: Modified Int32 attribute to new value: %d"), __FUNCTION__, NewValue);
	}
}
