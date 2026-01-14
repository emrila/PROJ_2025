// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UpgradeSystemInterface.generated.h"

class UUpgradeComponent;
// This class does not need to be modified.
UINTERFACE()
class UUpgradeSystemInterface : public UInterface
{
	GENERATED_BODY()
};

class UPGRADE_API IUpgradeSystemInterface
{
	GENERATED_BODY()

public:
	virtual UUpgradeComponent* GetUpgradeComponent() const = 0;
};
