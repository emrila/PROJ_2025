// 🐲Furkan approves of this🐲

#pragma once

#include "CoreMinimal.h"
#include "Components/SlateWrapperTypes.h"
#include "GameFramework/HUD.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"

UCLASS()
class GUI_API APlayerHUD : public AHUD
{
	GENERATED_BODY()
	
protected:
	
	virtual void BeginPlay() override;	
	
	void InitWidgets();
	void AddGameVersionToHUD();

	template <typename T = UUserWidget>
	T* CreateAndAddToViewPort(const TSubclassOf<T>& WidgetClass, bool Visible = true);

	template <typename T = UUserWidget>
	void ToggleMenuWidget(T* Widget);

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TSubclassOf<UUserWidget> HUDClass;
	
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TObjectPtr<UUserWidget> HUDWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TSubclassOf<UUserWidget> MenuClass;
	
	UPROPERTY(BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TObjectPtr<UUserWidget> MenuWidget;
	
	TSharedPtr<class STextSWidget> BuildVersionSlateWidget;
	TSharedPtr<class SWeakWidget> BuildVersionWidget;
};

template <typename T>
T* APlayerHUD::CreateAndAddToViewPort(const TSubclassOf<T>& WidgetClass, const bool Visible)
{
	if (!WidgetClass)
	{
		return nullptr;
	}

	T* Widget = CreateWidget<T>(GetWorld(), WidgetClass);

	if (!Widget)
	{
		return nullptr;
	}
	
	Widget->AddToViewport();	

	ESlateVisibility Visibility = Visible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	Widget->SetVisibility(Visibility);
	return Widget;
}

template <typename T>
void APlayerHUD::ToggleMenuWidget(T* Widget)
{
	if (!Widget)
	{
		return;
	}
	if (Widget->GetVisibility() == ESlateVisibility::Visible)
	{
		Widget->SetVisibility(ESlateVisibility::Collapsed);
		HUDWidget->SetVisibility(ESlateVisibility::Visible);

		APlayerController* Controller = GetOwningPlayerController();
		Controller->SetInputMode(FInputModeGameOnly());
		Controller->FlushPressedKeys();
		Controller->SetIgnoreMoveInput(false);
		Controller->SetShowMouseCursor(false);
	}
	else if (Widget->GetVisibility() == ESlateVisibility::Collapsed)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
		HUDWidget->SetVisibility(ESlateVisibility::Collapsed);

		APlayerController* Controller = GetOwningPlayerController();
		Controller->SetInputMode(FInputModeGameAndUI());
		Controller->SetIgnoreMoveInput(true);
		Controller->FlushPressedKeys();
		Controller->SetShowMouseCursor(true);
	}
}
