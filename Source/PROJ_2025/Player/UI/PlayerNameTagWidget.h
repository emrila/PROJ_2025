// Furkan approves of this

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerNameTagWidget.generated.h"

UCLASS()
class PROJ_2025_API UPlayerNameTagWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetCustomPlayerName(const FText& InPlayerName);
	void SetCustomPlayerName_Implementation(const FText& InPlayerName){}
};
