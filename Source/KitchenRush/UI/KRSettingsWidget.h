// KRSettingsWidget.h
// [M4] Settings, from the main menu or the pause menu. Everything saves as soon as it changes.
// WBP_Settings is its Blueprint. (Master volume arrives with the audio, later.)

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KRSettingsWidget.generated.h"

class UButton;
class USlider;
class UTextBlock;

UCLASS(Abstract)
class KITCHENRUSH_API UKRSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	/** Mouse sensitivity, 0.2x to 3x. */
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> SensitivitySlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> SensitivityText;

	/** UI size, 0.75x to 2x (Application Scale). Applied when you let go of the slider. */
	UPROPERTY(meta = (BindWidget)) TObjectPtr<USlider> UIScaleSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> UIScaleText;

	/** Click to switch: Freeze / Slow. The text inside shows the current choice. */
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> StillModeButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> StillModeText;

	/** Click to switch: Clamped / Momentum. */
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> MoveModeButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> MoveModeText;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BackButton;

private:
	void RefreshTexts();

	UFUNCTION() void HandleSensitivityChanged(float Value);
	UFUNCTION() void HandleUIScaleChanged(float Value);
	UFUNCTION() void HandleUIScaleReleased();
	UFUNCTION() void HandleStillMode();
	UFUNCTION() void HandleMoveMode();
	UFUNCTION() void HandleBack();
};
