// KRSettingsWidget.cpp

#include "UI/KRSettingsWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Flow/KRProfileSubsystem.h"
#include "KitchenRushPlayerController.h"

void UKRSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	const UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this);

	// Slider ranges are set here, so you don't have to type them in the Designer.
	SensitivitySlider->SetMinValue(0.2f);
	SensitivitySlider->SetMaxValue(3.f);
	SensitivitySlider->SetStepSize(0.05f);
	SensitivitySlider->SetValue(Profile ? Profile->GetMouseSensitivity() : 1.f);

	UIScaleSlider->SetMinValue(0.75f);
	UIScaleSlider->SetMaxValue(2.f);
	UIScaleSlider->SetStepSize(0.05f);
	UIScaleSlider->SetValue(Profile ? Profile->GetUIScale() : 1.f);

	SensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UKRSettingsWidget::HandleSensitivityChanged);
	UIScaleSlider->OnValueChanged.AddUniqueDynamic(this, &UKRSettingsWidget::HandleUIScaleChanged);
	// The UI rescales under the mouse, so the new size is applied when you let go, not while dragging.
	UIScaleSlider->OnMouseCaptureEnd.AddUniqueDynamic(this, &UKRSettingsWidget::HandleUIScaleReleased);
	// Volume sliders, if WBP_Settings has them.
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetMinValue(0.f);
		MasterVolumeSlider->SetMaxValue(1.f);
		MasterVolumeSlider->SetStepSize(0.05f);
		MasterVolumeSlider->SetValue(Profile ? Profile->GetMasterVolume() : 1.f);
		MasterVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UKRSettingsWidget::HandleMasterVolumeChanged);
	}
	if (MusicVolumeSlider)
	{
		MusicVolumeSlider->SetMinValue(0.f);
		MusicVolumeSlider->SetMaxValue(1.f);
		MusicVolumeSlider->SetStepSize(0.05f);
		MusicVolumeSlider->SetValue(Profile ? Profile->GetMusicVolume() : 1.f);
		MusicVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UKRSettingsWidget::HandleMusicVolumeChanged);
	}
	if (SlowdownDelaySlider)
	{
		SlowdownDelaySlider->SetMinValue(0.f);
		SlowdownDelaySlider->SetMaxValue(1.f);
		SlowdownDelaySlider->SetStepSize(0.05f);
		SlowdownDelaySlider->SetValue(Profile ? Profile->GetSlowdownDelay() : 0.2f);
		SlowdownDelaySlider->OnValueChanged.AddUniqueDynamic(this, &UKRSettingsWidget::HandleSlowdownDelayChanged);
	}

	StillModeButton->OnClicked.AddUniqueDynamic(this, &UKRSettingsWidget::HandleStillMode);
	MoveModeButton->OnClicked.AddUniqueDynamic(this, &UKRSettingsWidget::HandleMoveMode);
	BackButton->OnClicked.AddUniqueDynamic(this, &UKRSettingsWidget::HandleBack);

	RefreshTexts();
}

void UKRSettingsWidget::RefreshTexts()
{
	const UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this);
	if (!Profile)
	{
		return;
	}
	if (SensitivityText)
	{
		SensitivityText->SetText(FText::FromString(FString::Printf(TEXT("SENSITIVITY: %.2fx"), SensitivitySlider->GetValue()))); // label
	}
	if (UIScaleText)
	{
		UIScaleText->SetText(FText::FromString(FString::Printf(TEXT("UI SCALE: %.2fx"), UIScaleSlider->GetValue()))); // label
	}
	// "MASTER VOLUME: 80%"
	if (MasterVolumeText && MasterVolumeSlider)
	{
		MasterVolumeText->SetText(FText::FromString(FString::Printf(TEXT("MASTER VOLUME: %d%%"), FMath::RoundToInt(MasterVolumeSlider->GetValue() * 100.f))));
	}
	if (MusicVolumeText && MusicVolumeSlider)
	{
		MusicVolumeText->SetText(FText::FromString(FString::Printf(TEXT("MUSIC VOLUME: %d%%"), FMath::RoundToInt(MusicVolumeSlider->GetValue() * 100.f))));
	}
	if (SlowdownDelayText && SlowdownDelaySlider)
	{
		SlowdownDelayText->SetText(FText::FromString(FString::Printf(TEXT("SLOWDOWN DELAY: %.2f s"), SlowdownDelaySlider->GetValue())));
	}
	StillModeText->SetText(Profile->GetStillMode() == EKRStillMode::Freeze
		? NSLOCTEXT("KitchenRush", "StillFreeze", "Standing still: Freeze")
		: NSLOCTEXT("KitchenRush", "StillSlow", "Standing still: Slow motion"));
	MoveModeText->SetText(Profile->GetMoveMode() == EKRMoveMode::Clamped
		? NSLOCTEXT("KitchenRush", "MoveClamped", "Walking: Normal speed")
		: NSLOCTEXT("KitchenRush", "MoveMomentum", "Walking: Momentum (speeds up)"));
}

void UKRSettingsWidget::HandleSensitivityChanged(float Value)
{
	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		Profile->SetMouseSensitivity(Value);
	}
	RefreshTexts();
}

void UKRSettingsWidget::HandleUIScaleChanged(float Value)
{
	RefreshTexts(); // just the number while dragging
}

void UKRSettingsWidget::HandleUIScaleReleased()
{
	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		Profile->SetUIScale(UIScaleSlider->GetValue());
	}
}

void UKRSettingsWidget::HandleMasterVolumeChanged(float Value)
{
	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		Profile->SetMasterVolume(Value);
	}
	RefreshTexts();
}

void UKRSettingsWidget::HandleMusicVolumeChanged(float Value)
{
	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		Profile->SetMusicVolume(Value);
	}
	RefreshTexts();
}

void UKRSettingsWidget::HandleSlowdownDelayChanged(float Value)
{
	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		Profile->SetSlowdownDelay(Value);
	}
	RefreshTexts();
}

void UKRSettingsWidget::HandleStillMode()
{
	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		const EKRStillMode Next = Profile->GetStillMode() == EKRStillMode::Freeze ? EKRStillMode::Slow : EKRStillMode::Freeze;
		Profile->SetStillMode(Next, GetWorld());
	}
	RefreshTexts();
}

void UKRSettingsWidget::HandleMoveMode()
{
	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		const EKRMoveMode Next = Profile->GetMoveMode() == EKRMoveMode::Clamped ? EKRMoveMode::Momentum : EKRMoveMode::Clamped;
		Profile->SetMoveMode(Next, GetWorld());
	}
	RefreshTexts();
}

void UKRSettingsWidget::HandleBack()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->CloseSettings();
	}
}
