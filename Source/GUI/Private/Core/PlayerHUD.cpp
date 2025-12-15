// 🐲Furkan approves of this🐲


#include "Core/PlayerHUD.h"

#include "Dev/GUILog.h"
#include "Widgets/SWeakWidget.h"
#include "Widgets/Slate/STextSWidget.h"

void APlayerHUD::BeginPlay()
{
	Super::BeginPlay();
	
	//InitWidgets();
	//AddGameVersionToHUD();
}

void APlayerHUD::InitWidgets_Implementation()
{
	AddGameVersionToHUD();
	if (HUDClass)
	{
		HUDWidget = CreateAndAddToViewPort<UUserWidget>(HUDClass);
	}
	if (MenuClass)
	{
		MenuWidget = CreateAndAddToViewPort<UUserWidget>(MenuClass, false);
	}
}

void APlayerHUD::ToggleWidget_Implementation(int32 WidgetGroup)
{
	ToggleMenuWidget<UUserWidget>(MenuWidget);
}

void APlayerHUD::AddGameVersionToHUD()
{
	if (!GEngine)
	{
		GUI_ERROR( TEXT("%hs: GEngine is null!"), __FUNCTION__);
		return;
	}
	
	FString ProjectVersion;
	
	const TCHAR* Section = TEXT("/Script/EngineSettings.GeneralProjectSettings");
	const TCHAR* Key = TEXT("ProjectVersion");	
	GConfig->GetString(Section,Key, ProjectVersion, GGameIni);

	FString VersionString;
#if UE_BUILD_SHIPPING //include the build date
	
	FDateTime Now = FDateTime::Now();
	FString BuildDate = Now.ToString(TEXT("%Y.%m.%d %H:%M:%S"));
	VersionString = FString::Printf(TEXT("%s\n%s"), *BuildDate, *ProjectVersion);
#else // Just show the version	
	VersionString = FString::Printf(TEXT("Version: %s"), *ProjectVersion);
#endif
	
	BuildVersionSlateWidget = SNew(STextSWidget).InTextContent(FText::FromString(VersionString));	
	GEngine->GameViewport->AddViewportWidgetContent(SAssignNew(BuildVersionWidget, SWeakWidget).PossiblyNullContent(BuildVersionSlateWidget.ToSharedRef()));	
}

void APlayerHUD::CreateAndAdd_Implementation(TSubclassOf<UUserWidget> WidgetClass, bool Visible)
{
	UUserWidget* Widget = CreateAndAddToViewPort<UUserWidget>(WidgetClass, Visible);
	
}

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
		if (HUDClass)
		{
			HUDWidget->SetVisibility(ESlateVisibility::Visible);
		}

		APlayerController* Controller = GetOwningPlayerController();
		Controller->SetInputMode(FInputModeGameOnly());
		Controller->FlushPressedKeys();
		Controller->SetIgnoreMoveInput(false);
		Controller->SetShowMouseCursor(false);
		GUI_DISPLAY(TEXT("%hs: HUDWidget: Collapsed | Widget: Visible"), __FUNCTION__);

	}
	else if (Widget->GetVisibility() == ESlateVisibility::Collapsed)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
		
		if (HUDClass)
		{
			HUDWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		

		APlayerController* Controller = GetOwningPlayerController();
		Controller->SetInputMode(FInputModeGameAndUI());
		Controller->SetIgnoreMoveInput(true);
		Controller->FlushPressedKeys();
		Controller->SetShowMouseCursor(true);
		GUI_DISPLAY(TEXT("%hs: HUDWidget: Visible | Widget: Collapsed"), __FUNCTION__);
	}	
}