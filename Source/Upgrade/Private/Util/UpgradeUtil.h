#pragma once

struct FAttributeBase;

//#define TABLE_PATH TEXT("/Game/Developer/Emma/DT_UpgradeData.DT_UpgradeData")

namespace UpgradeUtils
{
	FProperty* GetProperty(const UObject* Owner, const FName PropertyName);

	bool IsValidProperty(const FProperty* Property);

	inline FString GetClassNameKey(const UObject* Object)
	{
		return FString::Printf(TEXT("%s_%s"), *Object->GetClass()->GetName(), *Object->GetName());
	}

	TUniquePtr<FAttributeBase> CreateAttribute(UObject* InOwner, FProperty* InProperty, const FName InRowName);

	inline uint64 GetKey(UObject* Owner, FProperty* Property)
	{
		return reinterpret_cast<uint64>(Owner) << 32 ^ reinterpret_cast<uint64>(Property);
	}
}
