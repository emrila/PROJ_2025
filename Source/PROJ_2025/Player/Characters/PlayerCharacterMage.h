
#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacterBase.h"
#include "PlayerCharacterMage.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(MageLog, Log, All);

UCLASS()
class PROJ_2025_API APlayerCharacterMage : public APlayerCharacterBase
{
	GENERATED_BODY()

public:
	APlayerCharacterMage();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	virtual void UseFirstAttackComponent() override;

	
};
