// KRSettingsWidget.h
// Settings, from the main menu or the pause menu. Everything saves as soon as it changes.
// WBP_Settings is its Blueprint. Optional volume sliders: add them to WBP_Settings if you want them.

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

	/** Optional: Sliders named MasterVolumeSlider / MusicVolumeSlider (and Texts for their labels). 0% to 100%. */
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> MasterVolumeSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> MasterVolumeText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<USlider> MusicVolumeSlider;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> MusicVolumeText;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> BackButton;

private:
	void RefreshTexts();

	UFUNCTION() void HandleSensitivityChanged(float Value);
	UFUNCTION() void HandleUIScaleChanged(float Value);
	UFUNCTION() void HandleUIScaleReleased();
	UFUNCTION() void HandleMasterVolumeChanged(float Value);
	UFUNCTION() void HandleMusicVolumeChanged(float Value);
	UFUNCTION() void HandleBack();
};
