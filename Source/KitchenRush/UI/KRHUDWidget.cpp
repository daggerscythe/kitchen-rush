// KRHUDWidget.cpp

#include "UI/KRHUDWidget.h"
#include "UI/KRTicketWidget.h"
#include "UI/KRReceiptLineWidget.h"
#include "Components/VerticalBoxSlot.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "Player/KRHealthComponent.h"
#include "Hazards/KRGusDirector.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interaction/KRInteractionComponent.h"
#include "Interaction/KRItem.h"
#include "Time/KRTimeSubsystem.h"
#include "KitchenRushGameMode.h"

void UKRHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// ---- Prompt and held item ----
	FText PromptLine = FText::GetEmpty();
	FText HeldLine = FText::GetEmpty();
	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		if (const UKRInteractionComponent* Interaction = Pawn->FindComponentByClass<UKRInteractionComponent>())
		{
			const FText Prompt = Interaction->GetPrompt();
			if (!Prompt.IsEmpty())
			{
				PromptLine = FText::Format(NSLOCTEXT("KitchenRush", "PromptLine", "[E] {0}"), Prompt);
			}
			if (const AKRItem* Held = Interaction->GetHeldItem())
			{
				HeldLine = FText::Format(NSLOCTEXT("KitchenRush", "HeldLine", "Holding: {0}"), Held->GetItemName());
			}
		}
	}
	PromptText->SetText(PromptLine);
	if (HeldText)
	{
		HeldText->SetText(HeldLine);
	}

	// ---- Pocket watch ----
	const UWorld* World = GetWorld();
	const UKRTimeSubsystem* Time = World ? World->GetSubsystem<UKRTimeSubsystem>() : nullptr;
	if (Time)
	{
		const float Scale = Time->GetTimeScale();
		HandAngle = FMath::Fmod(HandAngle + InDeltaTime * Scale * HandDegreesPerWorldSecond, 360.f);
		WatchHand->SetRenderTransformAngle(HandAngle);
		if (SpeedText)
		{
			SpeedText->SetText(FText::FromString(FString::Printf(TEXT("x%.2f"), Scale)));
		}
	}

	UpdateClock();
	UpdateOrders();
	UpdateHealth(); // [M3]
	UpdateGusWarning(); // [M3.5]
	UpdateReceiptLog(); // [M3.7]
}

// [M3.7]
void UKRHUDWidget::UpdateReceiptLog()
{
	UWorld* World = GetWorld();
	const AKitchenRushGameMode* GameMode = World ? World->GetAuthGameMode<AKitchenRushGameMode>() : nullptr;
	if (!GameMode || !ReceiptLog || !ReceiptLineClass)
	{
		return;
	}
	const double Now = World->GetRealTimeSeconds(); // real time: the log animates even while you're frozen

	// 1) Queue the lines of any new receipts, one LineInterval apart, after anything still queued.
	for (const FKRReceipt& Receipt : GameMode->GetReceipts())
	{
		if (Receipt.Id <= LastReceiptId)
		{
			continue;
		}
		LastReceiptId = Receipt.Id;
		double ShowAt = FMath::Max(Now, NextLineTime);
		const double LastShow = ShowAt + LineInterval * (Receipt.Lines.Num() - 1);
		for (const FKRReceiptLine& Line : Receipt.Lines)
		{
			FLogLine Entry;
			Entry.Line = Line;
			Entry.ShowAt = ShowAt;
			Entry.HideAt = LastShow + ReceiptVisibleSeconds; // the whole receipt leaves together
			LogLines.Add(Entry);
			ShowAt += LineInterval;
		}
		NextLineTime = ShowAt + LineInterval; // a small gap before the next receipt
	}

	// 2) Remove receipts whose time is up (walk backwards so removing doesn't skip anything).
	for (int32 i = LogLines.Num() - 1; i >= 0; --i)
	{
		if (Now >= LogLines[i].HideAt)
		{
			if (UKRReceiptLineWidget* Old = LogLines[i].Widget.Get())
			{
				Old->RemoveFromParent();
			}
			LogLines.RemoveAt(i);
		}
	}

	// 3) Show and animate, oldest first, so lines land in the box in the right order.
	for (FLogLine& Entry : LogLines)
	{
		if (Now < Entry.ShowAt)
		{
			break; // lines are queued in time order, so everything after this is waiting too
		}
		UKRReceiptLineWidget* Widget = Entry.Widget.Get();
		if (!Widget)
		{
			Widget = CreateWidget<UKRReceiptLineWidget>(GetOwningPlayer(), ReceiptLineClass);
			if (!Widget)
			{
				continue;
			}
			Widget->Setup(Entry.Line);
			ReceiptLog->AddChildToVerticalBox(Widget);
			Entry.Widget = Widget;
		}
		// Slide in from the left with an ease-out (fast, then settling): 1 - (1 - t)^3.
		const float T = FMath::Clamp(float((Now - Entry.ShowAt) / FMath::Max(SlideSeconds, 0.01f)), 0.f, 1.f);
		const float Ease = 1.f - FMath::Pow(1.f - T, 3.f);
		Widget->SetRenderTranslation(FVector2D(-SlideDistance * (1.f - Ease), 0.f));
		// Fade in while sliding; fade out over the last half second.
		const float FadeOut = FMath::Clamp(float((Entry.HideAt - Now) / 0.5), 0.f, 1.f);
		Widget->SetRenderOpacity(Ease * FadeOut);
	}
}

// [M3.5]
void UKRHUDWidget::UpdateGusWarning()
{
	const AKRGusDirector* Gus = AKRGusDirector::Find(GetWorld());
	const bool bWarning = Gus && Gus->IsWarning();

	if (GusWarningText)
	{
		GusWarningText->SetVisibility(bWarning ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bWarning)
		{
			GusWarningText->SetText(Gus->GetWarningText());
			// Pulse in REAL time so it keeps throbbing while you're frozen.
			const float Pulse = 1.f + 0.08f * FMath::Sin(float(GetWorld()->GetRealTimeSeconds()) * 9.f);
			GusWarningText->SetRenderScale(FVector2D(Pulse, Pulse));
		}
	}
	else if (bWarning && MessageText)
	{
		MessageText->SetText(Gus->GetWarningText());
	}

	if (!GusArrow)
	{
		return;
	}
	bool bShowArrow = false;
	const APlayerController* PC = GetOwningPlayer();
	if (bWarning && PC && PC->PlayerCameraManager)
	{
		// Where is Gus relative to where we're looking? Unrotate the direction into "camera space":
		// X = ahead, Y = right. Atan2 turns that into an angle: 0 = straight ahead, 90 = to the right.
		const FVector ToGus = Gus->GetActorLocation() - PC->PlayerCameraManager->GetCameraLocation();
		const FVector Local = FRotator(0.f, PC->PlayerCameraManager->GetCameraRotation().Yaw, 0.f).UnrotateVector(ToGus);
		const float Angle = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X));
		bShowArrow = FMath::Abs(Angle) > GusOnScreenAngle;
		GusArrow->SetRenderTransformAngle(Angle);
	}
	GusArrow->SetVisibility(bShowArrow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

// [M3]
void UKRHUDWidget::UpdateHealth()
{
	const APawn* Pawn = GetOwningPlayerPawn();
	const UKRHealthComponent* Health = Pawn ? Pawn->FindComponentByClass<UKRHealthComponent>() : nullptr;
	if (!Health)
	{
		return;
	}
	if (HealthBar)
	{
		const float Percent = Health->GetHealthPercent();
		HealthBar->SetPercent(Percent);
		// Green when healthy, orange when hurt, red when in danger.
		const FLinearColor Color = Percent > 0.6f ? FLinearColor(0.3f, 0.85f, 0.4f)
			: Percent > 0.3f ? FLinearColor(1.f, 0.6f, 0.2f)
			: FLinearColor(0.9f, 0.2f, 0.2f);
		HealthBar->SetFillColorAndOpacity(Color);
	}
	if (DamageFlash)
	{
		DamageFlash->SetRenderOpacity(Health->GetDamageFlashAlpha());
	}
}

void UKRHUDWidget::UpdateClock()
{
	const UWorld* World = GetWorld();
	const UKRTimeSubsystem* Time = World ? World->GetSubsystem<UKRTimeSubsystem>() : nullptr;
	if (!ClockText || !Time)
	{
		return;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);
	const float GameHours = T->ShiftStartHour + Time->GetWorldSecondsElapsed() / T->WorldSecondsPerGameHour;
	const int32 Hour24 = FMath::FloorToInt(GameHours) % 24;
	const int32 Minute = FMath::FloorToInt(FMath::Frac(GameHours) * 60.f);
	int32 Hour12 = Hour24 % 12;
	if (Hour12 == 0)
	{
		Hour12 = 12;
	}
	ClockText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d %s"), Hour12, Minute, Hour24 < 12 ? TEXT("AM") : TEXT("PM"))));
}

void UKRHUDWidget::UpdateOrders()
{
	UWorld* World = GetWorld();
	const AKitchenRushGameMode* GameMode = World ? World->GetAuthGameMode<AKitchenRushGameMode>() : nullptr;
	if (!GameMode)
	{
		return;
	}

	if (TipsText)
	{
		TipsText->SetText(FText::FromString(FString::Printf(TEXT("$%d"), GameMode->GetTips())));
	}
	if (MessageText)
	{
		MessageText->SetText(GameMode->GetMessage());
	}

	// Strikes: only swap textures when the count changes.
	if (GameMode->GetStrikes() != ShownStrikes)
	{
		ShownStrikes = GameMode->GetStrikes();
		UImage* StrikeImages[] = { Strike0, Strike1, Strike2 };
		for (int32 i = 0; i < 3; ++i)
		{
			UTexture2D* Texture = (i < ShownStrikes) ? StrikeBrokenTexture : StrikeEmptyTexture;
			if (StrikeImages[i] && Texture)
			{
				StrikeImages[i]->SetBrushFromTexture(Texture);
			}
		}
	}

	// Tickets: rebuild the list only when the set of orders changes, then update patience bars.
	if (!TicketList || !TicketWidgetClass)
	{
		return;
	}
	const TArray<FKROrder>& Orders = GameMode->GetOrders();
	TArray<int32> Ids;
	for (const FKROrder& Order : Orders)
	{
		Ids.Add(Order.Id);
	}
	if (Ids != ShownOrderIds)
	{
		TicketList->ClearChildren();
		TicketWidgets.Empty();
		for (const FKROrder& Order : Orders)
		{
			UKRTicketWidget* Ticket = CreateWidget<UKRTicketWidget>(GetOwningPlayer(), TicketWidgetClass);
			if (!Ticket)
			{
				continue;
			}
			if (const FKRRecipeRow* Recipe = GameMode->FindRecipe(Order.RecipeRow))
			{
				Ticket->Setup(*Recipe);
			}
			TicketList->AddChildToVerticalBox(Ticket);
			TicketWidgets.Add(Ticket);
		}
		ShownOrderIds = Ids;
	}
	// (If a ticket widget failed to create, the lists can differ in length; stop at the shorter.)
	for (int32 i = 0; i < Orders.Num() && i < TicketWidgets.Num(); ++i)
	{
		TicketWidgets[i]->SetPatience(Orders[i].PatienceLeft / Orders[i].PatienceTotal);
	}
}
