// KRPauseMenuWidget.cpp

#include "UI/KRPauseMenuWidget.h"
#include "Components/Button.h"
#include "KitchenRushPlayerController.h"

void UKRPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ResumeButton->OnClicked.AddUniqueDynamic(this, &UKRPauseMenuWidget::HandleResume);
	SettingsButton->OnClicked.AddUniqueDynamic(this, &UKRPauseMenuWidget::HandleSettings);
	RestartActButton->OnClicked.AddUniqueDynamic(this, &UKRPauseMenuWidget::HandleRestartAct);
	MainMenuButton->OnClicked.AddUniqueDynamic(this, &UKRPauseMenuWidget::HandleMainMenu);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UKRPauseMenuWidget::HandleQuit);
}

void UKRPauseMenuWidget::HandleResume()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->TogglePause();
	}
}

void UKRPauseMenuWidget::HandleSettings()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->OpenSettings();
	}
}

void UKRPauseMenuWidget::HandleRestartAct()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->RestartCurrentAct();
	}
}

void UKRPauseMenuWidget::HandleMainMenu()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->GoToMainMenu();
	}
}

void UKRPauseMenuWidget::HandleQuit()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->QuitToDesktop();
	}
}
