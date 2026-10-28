// KRHUDWidget.h
// The C++ half of the HUD.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KRHUDWidget.generated.h"

class UTextBlock;
class UImage;
class UVerticalBox;
class UTexture2D;
class UKRTicketWidget;

UCLASS(Abstract)
class KITCHENRUSH_API UKRHUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Runs every frame while the widget is on screen. InDeltaTime is REAL time. */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PromptText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WatchHand;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeedText;

	UPROPERTY(EditAnywhere, Category = "Watch")
	float HandDegreesPerWorldSecond = 60.f;


	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TipsText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ClockText;

	/** "Holding: Plate (Chopped Tomato)" */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HeldText;

	/** Short messages: tips earned, strikes, soup boiled over. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MessageText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Strike0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Strike1;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Strike2;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> TicketList;

	UPROPERTY(EditAnywhere, Category = "Orders")
	TSubclassOf<UKRTicketWidget> TicketWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Orders")
	TObjectPtr<UTexture2D> StrikeEmptyTexture;

	UPROPERTY(EditAnywhere, Category = "Orders")
	TObjectPtr<UTexture2D> StrikeBrokenTexture;

private:
	void UpdateOrders();
	void UpdateClock();

	float HandAngle = 0.f;

	/** Tickets shown in the same order as their order IDs. */
	UPROPERTY()
	TArray<TObjectPtr<UKRTicketWidget>> TicketWidgets;

	TArray<int32> ShownOrderIds;
	int32 ShownStrikes = -1;
};
