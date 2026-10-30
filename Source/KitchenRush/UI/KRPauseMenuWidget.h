// KRPauseMenuWidget.h
// [M4] Esc / P during play. WBP_PauseMenu is its Blueprint.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KRPauseMenuWidget.generated.h"

class UButton;

UCLASS(Abstract)
class KITCHENRUSH_API UKRPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ResumeButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> SettingsButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> RestartActButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> MainMenuButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> QuitButton;

private:
	UFUNCTION() void HandleResume();
	UFUNCTION() void HandleSettings();
	UFUNCTION() void HandleRestartAct();
	UFUNCTION() void HandleMainMenu();
	UFUNCTION() void HandleQuit();
};
