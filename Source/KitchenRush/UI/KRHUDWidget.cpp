// KRHUDWidget.cpp

#include "UI/KRHUDWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Interaction/KRInteractionComponent.h"
#include "Time/KRTimeSubsystem.h"

void UKRHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// ---- Prompt under the crosshair ----
	FText PromptLine = FText::GetEmpty();
	if (APawn* Pawn = GetOwningPlayerPawn())
	{
		if (const UKRInteractionComponent* Interaction = Pawn->FindComponentByClass<UKRInteractionComponent>())
		{
			const FText Prompt = Interaction->GetPrompt();
			if (!Prompt.IsEmpty())
			{
				PromptLine = FText::Format(NSLOCTEXT("KitchenRush", "PromptLine", "[E] {0}"), Prompt);
			}
		}
	}
	PromptText->SetText(PromptLine);

	// ---- Pocket watch: the hand turns with WORLD time, so it stops when you freeze ----
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
}
