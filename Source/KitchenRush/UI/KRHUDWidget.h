// KRHUDWidget.h
// The C++ half of the HUD

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KRHUDWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS(Abstract)
class KITCHENRUSH_API UKRHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Runs every frame while the widget is on screen. InDeltaTime is REAL time */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Text under the crosshair */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PromptText;

	/** The pocket watch's hand */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WatchHand;

	/** Optional "x1.00" readout */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeedText;

	/** How fast the watch hand turns, in degrees per second of WORLD time */
	UPROPERTY(EditAnywhere, Category = "Watch")
	float HandDegreesPerWorldSecond = 60.f;

private:
	float HandAngle = 0.f;
};
