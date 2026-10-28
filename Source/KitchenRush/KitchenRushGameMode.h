// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Cooking/KRTypes.h"
#include "KitchenRushGameMode.generated.h"

class AKRPlate;
class UDataTable;
class UKRTuningData;

/**
 *  Simple GameMode for a first person game
 */
UCLASS(abstract)
class AKitchenRushGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AKitchenRushGameMode();

	virtual void Tick(float DeltaSeconds) override;

	/** DA_Tuning, or nullptr if not set (use UKRTuningData::Get instead of calling this directly). */
	const UKRTuningData* GetTuning() const;

	/** Look up a recipe by its row name in DT_Recipes. */
	const FKRRecipeRow* FindRecipe(FName RowName) const;

	const TArray<FKROrder>& GetOrders() const { return Orders; }
	int32 GetTips() const { return Tips; }
	int32 GetStrikes() const { return Strikes; }
	bool IsGameOver() const { return bGameOver; }

	/** Serve a plate/bowl to the best-matching open ticket. Returns false if there are no orders. */
	bool ServeDish(AKRPlate* Plate);

	/** Show a short message on the HUD for a few seconds ("+$14  Garden Salad"). */
	void ShowMessage(const FText& Text);

	/** The current message, or empty text if it has expired. */
	FText GetMessage() const;

	/** Toggle orders */
	UFUNCTION(BlueprintCallable, Category = "Kitchen Rush")
	void SetOrdersEnabled(bool bEnabled) { bOrdersEnabled = bEnabled; }

protected:
	virtual void BeginPlay() override;

	/** Set to DA_Tuning in BP_FirstPersonGameMode. */
	UPROPERTY(EditDefaultsOnly, Category = "Kitchen Rush")
	TObjectPtr<UKRTuningData> Tuning;

	/** Set to DT_Recipes in BP_FirstPersonGameMode. */
	UPROPERTY(EditDefaultsOnly, Category = "Kitchen Rush")
	TObjectPtr<UDataTable> RecipeTable;

	UPROPERTY(EditAnywhere, Category = "Kitchen Rush")
	bool bOrdersEnabled = true;

private:
	void SpawnOrder();
	void AddStrike(const FText& Reason);

	/** 0..1 correctness of Plate for Recipe; also reports whether every cooked part was perfect. */
	float ScorePlate(const FKRRecipeRow& Recipe, const AKRPlate* Plate, bool& bOutPerfect) const;

	TArray<FKROrder> Orders;
	int32 NextOrderId = 1;
	float SecondsUntilNextOrder = 0.f;
	int32 Tips = 0;
	int32 Strikes = 0;
	bool bGameOver = false;

	FText Message;
	double MessageExpiresAt = 0.0; // real time, so messages fade even while frozen
};
