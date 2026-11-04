// KRNoteCardWidget.h
// One of Nana's handwritten notes. The game pauses while it's up.
// "Got it" closes it; "Skip tutorial" (top-left) closes it and turns the tutorial off for this run.
// WBP_NoteCard is its Blueprint: T_NoteCard as the background, the Caveat font for the words.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KRNoteCardWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS(Abstract)
class KITCHENRUSH_API UKRNoteCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(const FText& Title, const FText& Body);
	void SetupBrowser(const FText& Title, const TArray<FText>& InPages);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> TitleText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> BodyText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> GotItButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> SkipTutorialButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> PrevButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UButton> NextButton;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> PageText;

private:
	UFUNCTION() void HandleGotIt();
	UFUNCTION() void HandleSkip();
	UFUNCTION() void HandlePrev();
	UFUNCTION() void HandleNext();
	void ShowPage();
	bool bBrowsing = false;
	TArray<FText> Pages;
	int32 Page = 0;
};
