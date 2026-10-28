// KRTicketWidget.cpp 

#include "UI/KRTicketWidget.h"
#include "Cooking/KRTypes.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UKRTicketWidget::Setup(const FKRRecipeRow& Recipe)
{
	DishName->SetText(Recipe.DisplayName);

	if (DishIcon && Recipe.Icon)
	{
		DishIcon->SetBrushFromTexture(Recipe.Icon);
	}

	if (PartsText)
	{
		// UEnum::GetDisplayValueAsText reads the UMETA(DisplayName = ...) from KRTypes.h.
		TArray<FString> Lines;
		for (const EKRDishPart Part : Recipe.Parts)
		{
			Lines.Add(UEnum::GetDisplayValueAsText(Part).ToString());
		}
		if (Recipe.bServedInBowl)
		{
			Lines.Add(TEXT("(in a bowl)"));
		}
		PartsText->SetText(FText::FromString(FString::Join(Lines, TEXT("\n"))));
	}
}

void UKRTicketWidget::SetPatience(float Fraction)
{
	Fraction = FMath::Clamp(Fraction, 0.f, 1.f);
	PatienceBar->SetPercent(Fraction);

	FLinearColor Color(0.3f, 0.85f, 0.4f);                               // green
	if (Fraction < 0.25f)      Color = FLinearColor(0.9f, 0.2f, 0.2f);   // red
	else if (Fraction < 0.5f)  Color = FLinearColor(1.f, 0.8f, 0.2f);    // yellow
	PatienceBar->SetFillColorAndOpacity(Color);
}
