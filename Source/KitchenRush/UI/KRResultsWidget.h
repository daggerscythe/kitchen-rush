// KRResultsWidget.h
// [M4] End of the night: "Shift complete!" with stars, or "Game Over". WBP_Results is its Blueprint.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Flow/KRFlowTypes.h"
#include "KRResultsWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

UCLASS(Abstract)
class KITCHENRUSH_API UKRResultsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fill in the screen. Called just before it's shown. */
	void Setup(const FKRResults& Results);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TitleText;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TipsText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> ReasonText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> BestText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UTextBlock> StatsText;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Star0;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Star1;
	UPROPERTY(meta = (BindWidgetOptional)) TObjectPtr<UImage> Star2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> PlayAgainButton;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> MainMenuButton;

	/** T_Star_Full and T_Star_Empty. */
	UPROPERTY(EditAnywhere, Category = "Results") TObjectPtr<UTexture2D> StarFullTexture;
	UPROPERTY(EditAnywhere, Category = "Results") TObjectPtr<UTexture2D> StarEmptyTexture;

private:
	UFUNCTION() void HandlePlayAgain();
	UFUNCTION() void HandleMainMenu();
};
