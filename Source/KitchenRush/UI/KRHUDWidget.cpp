// KRHUDWidget.cpp

#include "UI/KRHUDWidget.h"
#include "UI/KRTicketWidget.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
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
