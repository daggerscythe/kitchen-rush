// KRMainMenuWidget.cpp

#include "UI/KRMainMenuWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Flow/KRProfileSubsystem.h"
#include "KitchenRushPlayerController.h"

void UKRMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayButton->OnClicked.AddUniqueDynamic(this, &UKRMainMenuWidget::HandlePlay);
	ContinueButton->OnClicked.AddUniqueDynamic(this, &UKRMainMenuWidget::HandleContinue);
	SettingsButton->OnClicked.AddUniqueDynamic(this, &UKRMainMenuWidget::HandleSettings);
	QuitButton->OnClicked.AddUniqueDynamic(this, &UKRMainMenuWidget::HandleQuit);

	// Continue only makes sense once you've reached Act 2.
	const UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this);
	const int32 LastAct = Profile ? Profile->GetLastActReached() : 1;
	ContinueButton->SetIsEnabled(LastAct >= 2);
	if (ContinueText)
	{
		ContinueText->SetText(LastAct >= 2
			? FText::Format(NSLOCTEXT("KitchenRush", "ContinueAct", "Continue (Act {0})"), LastAct)
			: NSLOCTEXT("KitchenRush", "Continue", "Continue"));
	}
	if (BestText)
	{
		const int32 Best = Profile ? Profile->GetHighScore() : 0;
		BestText->SetText(Best > 0
			? FText::Format(NSLOCTEXT("KitchenRush", "BestTips", "Best night: ${0}"), Best)
			: FText::GetEmpty());
	}
}

void UKRMainMenuWidget::HandlePlay()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->PlayFromMenu();
	}
}

void UKRMainMenuWidget::HandleContinue()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->ContinueFromMenu();
	}
}

void UKRMainMenuWidget::HandleSettings()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->OpenSettings();
	}
}

void UKRMainMenuWidget::HandleQuit()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->QuitToDesktop();
	}
}
