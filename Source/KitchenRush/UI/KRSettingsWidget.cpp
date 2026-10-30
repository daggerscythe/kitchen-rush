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
		SensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.2fx"), SensitivitySlider->GetValue())));
	}
	if (UIScaleText)
	{
		UIScaleText->SetText(FText::FromString(FString::Printf(TEXT("%.2fx"), UIScaleSlider->GetValue())));
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
