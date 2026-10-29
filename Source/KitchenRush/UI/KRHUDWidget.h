// KRHUDWidget.h
// The C++ half of the HUD.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Cooking/KRTypes.h"
#include "KRHUDWidget.generated.h"

class UTextBlock;
class UImage;
class UProgressBar;
class UVerticalBox;
class UTexture2D;
class UKRTicketWidget;
class UKRReceiptLineWidget;

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

	/** [M3] Top-left health bar. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	/** [M3] Full-screen red image that flashes when you're hurt. Set its colour to red with low alpha. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> DamageFlash;

	/** [M3.5] Big pulsing warning before Gus tosses or trips. Without it, the warning goes in MessageText. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GusWarningText;

	/** [M3.5] Arrow (pointing UP in the texture) that turns to point at Gus during a warning, when he's off-screen. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> GusArrow;

	/** [M3.5] Gus counts as "on screen" within this many degrees of where you look; outside it the arrow shows. */
	UPROPERTY(EditAnywhere, Category = "Gus Warning")
	float GusOnScreenAngle = 40.f;

	// ---- [M3.7] Receipt log: served orders, line by line, on the left side of the screen ----

	/** A Vertical Box on the left-centre of the screen. Receipt lines are added to it. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ReceiptLog;

	/** WBP_ReceiptLine. */
	UPROPERTY(EditAnywhere, Category = "Receipt Log")
	TSubclassOf<UKRReceiptLineWidget> ReceiptLineClass;

	/** Real seconds between lines appearing (the "ka-ching, ka-ching" rhythm). */
	UPROPERTY(EditAnywhere, Category = "Receipt Log")
	float LineInterval = 0.25f;

	/** Real seconds each line takes to slide in. */
	UPROPERTY(EditAnywhere, Category = "Receipt Log")
	float SlideSeconds = 0.2f;

	/** How far left (px) a line starts before sliding in. */
	UPROPERTY(EditAnywhere, Category = "Receipt Log")
	float SlideDistance = 450.f;

	/** Real seconds a receipt stays after its last line appears (then it fades out). */
	UPROPERTY(EditAnywhere, Category = "Receipt Log")
	float ReceiptVisibleSeconds = 10.f;

private:
	void UpdateOrders();
	void UpdateClock();
	void UpdateHealth(); // [M3]
	void UpdateGusWarning(); // [M3.5]
	void UpdateReceiptLog(); // [M3.7]

	/** [M3.7] A receipt line waiting to appear, or on screen. */
	struct FLogLine
	{
		FKRReceiptLine Line;
		double ShowAt = 0.0;   // real time it slides in
		double HideAt = 0.0;   // real time it's gone
		TWeakObjectPtr<UKRReceiptLineWidget> Widget; // null until it appears (the Vertical Box keeps it alive)
	};
	TArray<FLogLine> LogLines;
	int32 LastReceiptId = 0;
	double NextLineTime = 0.0;

	float HandAngle = 0.f;

	/** Tickets shown in the same order as their order IDs. */
	UPROPERTY()
	TArray<TObjectPtr<UKRTicketWidget>> TicketWidgets;

	TArray<int32> ShownOrderIds;
	int32 ShownStrikes = -1;
};
