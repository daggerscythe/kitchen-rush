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
	if (PrevButton) { PrevButton->OnClicked.AddUniqueDynamic(this, &UKRNoteCardWidget::HandlePrev); }
	if (NextButton) { NextButton->OnClicked.AddUniqueDynamic(this, &UKRNoteCardWidget::HandleNext); }
}

void UKRNoteCardWidget::Setup(const FText& Title, const FText& Body)
{
	if (TitleText)
	{
		TitleText->SetText(Title);
	}
	BodyText->SetText(Body);
	bBrowsing = false;
	SkipTutorialButton->SetVisibility(ESlateVisibility::Visible);
	if (PrevButton) { PrevButton->SetVisibility(ESlateVisibility::Collapsed); }
	if (NextButton) { NextButton->SetVisibility(ESlateVisibility::Collapsed); }
	if (PageText) { PageText->SetVisibility(ESlateVisibility::Collapsed); }
}

void UKRNoteCardWidget::SetupBrowser(const FText& Title, const TArray<FText>& InPages)
{
	if (TitleText) { TitleText->SetText(Title); }
	bBrowsing = true;
	Pages = InPages;
	Page = FMath::Max(Pages.Num() - 1, 0); // open on the newest note
	SkipTutorialButton->SetVisibility(ESlateVisibility::Collapsed);
	if (PrevButton) { PrevButton->SetVisibility(ESlateVisibility::Visible); }
	if (NextButton) { NextButton->SetVisibility(ESlateVisibility::Visible); }
	if (PageText) { PageText->SetVisibility(ESlateVisibility::Visible); }
	ShowPage();
}

void UKRNoteCardWidget::ShowPage()
{
	if (Pages.Num() == 0)
	{
		BodyText->SetText(NSLOCTEXT("KitchenRush", "NoNotesYet", "No notes yet."));
		if (PageText) { PageText->SetText(FText::GetEmpty()); }
		return;
	}
	Page = FMath::Clamp(Page, 0, Pages.Num() - 1);
	BodyText->SetText(Pages[Page]);
	if (PageText) { PageText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Page + 1, Pages.Num()))); }
	if (PrevButton) { PrevButton->SetIsEnabled(Page > 0); }
	if (NextButton) { NextButton->SetIsEnabled(Page < Pages.Num() - 1); }
}

void UKRNoteCardWidget::HandlePrev() { --Page; ShowPage(); }
void UKRNoteCardWidget::HandleNext() { ++Page; ShowPage(); }

void UKRNoteCardWidget::HandleGotIt()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		if (bBrowsing) { PC->CloseNotes(); }
		else { PC->CloseNote(false); }
	}
}

void UKRNoteCardWidget::HandleSkip()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->CloseNote(true);
	}
}
