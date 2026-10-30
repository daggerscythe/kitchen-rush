// KRNoteCardWidget.cpp

#include "UI/KRNoteCardWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "KitchenRushPlayerController.h"

void UKRNoteCardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GotItButton->OnClicked.AddUniqueDynamic(this, &UKRNoteCardWidget::HandleGotIt);
	SkipTutorialButton->OnClicked.AddUniqueDynamic(this, &UKRNoteCardWidget::HandleSkip);
}

void UKRNoteCardWidget::Setup(const FText& Title, const FText& Body)
{
	if (TitleText)
	{
		TitleText->SetText(Title);
	}
	BodyText->SetText(Body);
}

void UKRNoteCardWidget::HandleGotIt()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->CloseNote(false);
	}
}

void UKRNoteCardWidget::HandleSkip()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->CloseNote(true);
	}
}
