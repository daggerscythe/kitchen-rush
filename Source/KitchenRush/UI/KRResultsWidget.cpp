// KRResultsWidget.cpp

#include "UI/KRResultsWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "KitchenRushPlayerController.h"

void UKRResultsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PlayAgainButton->OnClicked.AddUniqueDynamic(this, &UKRResultsWidget::HandlePlayAgain);
	MainMenuButton->OnClicked.AddUniqueDynamic(this, &UKRResultsWidget::HandleMainMenu);
}

void UKRResultsWidget::Setup(const FKRResults& Results)
{
	TitleText->SetText(Results.bWon
		? NSLOCTEXT("KitchenRush", "ShiftComplete", "Shift complete!")
		: NSLOCTEXT("KitchenRush", "GameOverTitle", "Game Over"));
	TipsText->SetText(FText::Format(NSLOCTEXT("KitchenRush", "ResultsTips", "${0} in tips"), Results.Tips));

	if (ReasonText)
	{
		ReasonText->SetText(Results.Reason);
	}
	if (BestText)
	{
		BestText->SetText(Results.bNewBest
			? NSLOCTEXT("KitchenRush", "NewBest", "New best night!")
			: FText::Format(NSLOCTEXT("KitchenRush", "BestSoFar", "Best night: ${0}"), Results.Best));
	}
	if (StatsText)
	{
		StatsText->SetText(FText::Format(NSLOCTEXT("KitchenRush", "ResultsStats", "Orders served: {0}    Strikes: {1}"), Results.Served, Results.Strikes));
	}

	// Stars: full up to the count, empty after. Hidden entirely on Game Over.
	UImage* Stars[] = { Star0, Star1, Star2 };
	for (int32 i = 0; i < 3; ++i)
	{
		if (!Stars[i])
		{
			continue;
		}
		Stars[i]->SetVisibility(Results.bWon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		UTexture2D* Texture = (i < Results.Stars) ? StarFullTexture : StarEmptyTexture;
		if (Texture)
		{
			Stars[i]->SetBrushFromTexture(Texture);
		}
	}
}

void UKRResultsWidget::HandlePlayAgain()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->RestartAtAct(1); // a fresh night, tutorial included (skippable)
	}
}

void UKRResultsWidget::HandleMainMenu()
{
	if (AKitchenRushPlayerController* PC = Cast<AKitchenRushPlayerController>(GetOwningPlayer()))
	{
		PC->GoToMainMenu();
	}
}
