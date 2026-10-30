// KRMainMenuWidget.h
// [M4] The main menu, drawn over the frozen kitchen. WBP_MainMenu is its Blueprint.
// The buttons must be named exactly as below (PlayButton, ContinueButton...).

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KRMainMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS(Abstract)
class KITCHENRUSH_API UKRMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Runs once when the widget is created: hook up the buttons. */
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> PlayButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ContinueButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> SettingsButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> QuitButton;

	/** "Best: $214" (optional). */
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> BestText;

	/** "Continue (Act 2)" (optional: the text inside ContinueButton). */
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ContinueText;

private:
	// Button click handlers must be UFUNCTIONs so OnClicked.AddDynamic can find them.
	UFUNCTION() void HandlePlay();
	UFUNCTION() void HandleContinue();
	UFUNCTION() void HandleSettings();
	UFUNCTION() void HandleQuit();
};
