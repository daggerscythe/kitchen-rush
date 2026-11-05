// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Cooking/KRTypes.h"
#include "Flow/KRFlowTypes.h" // game events, run outcomes, results
#include "KitchenRushGameMode.generated.h"

class AKRPlate;

/** Everything ScorePlate works out about a served plate (not a UStruct: C++ only). */
struct FKRPlateScore
{
	float Correctness = 0.f;    // 0..1
	bool bPerfectOrder = false; // every part right, nothing extra
	bool bPerfectCook = false;  // every cooked part pulled in its perfect window
	int32 TrickShots = 0;       // matching parts only
	int32 GusAssists = 0;       // matching parts only
	int32 GroundTouches = 0;    // every part on the plate
};
class UDataTable;
class UKRTuningData;
class UKRAudioData;

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

	/** DA_Audio, or nullptr if not set (use UKRAudioData::Get instead of calling this directly). */
	const UKRAudioData* GetAudio() const;

	/** Look up a recipe by its row name in DT_Recipes. */
	const FKRRecipeRow* FindRecipe(FName RowName) const;

	const TArray<FKROrder>& GetOrders() const { return Orders; }
	int32 GetTips() const { return Tips; }
	int32 GetStrikes() const { return Strikes; }
	bool IsGameOver() const { return bGameOver; }

	/** Serve a plate/bowl to the best-matching open ticket. Returns false if there are no orders.
	 * bThrownIntoPass: it was thrown through the window (one more trick shot). Builds a receipt. */
	bool ServeDish(AKRPlate* Plate, bool bThrownIntoPass = false);

	/** Recent order receipts, oldest first. The HUD shows new ones line by line. */
	const TArray<FKRReceipt>& GetReceipts() const { return Receipts; }

	/** Show a short message on the HUD for a few seconds ("+$14  Garden Salad"). */
	void ShowMessage(const FText& Text, float Seconds = 3.f);

	/** Add tips outside of serving (dodging a knife) and show "+$1  Dodged!". */
	void AddBonus(int32 Dollars, const FText& Reason);

	/** End the night early: health hit 0 or 3 strikes. Shows the Game Over screen. */
	void TriggerGameOver(const FText& Reason, EKRRunOutcome Outcome = EKRRunOutcome::Health);

	// ---------------- Night flow (the act director drives these) ----------------

	/** Everyone who cares about "what just happened" (act director, playtest logger) binds to this. */
	FKROnGameEvent OnGameEvent;

	/** Tell everyone something happened. */
	void BroadcastEvent(EKRGameEvent Type, FName Name = NAME_None, float Value = 0.f, int32 Flags = 0);

	/** Same, from anywhere (items, hazards): finds the GameMode for you. Does nothing if there isn't one. */
	static void NotifyGameEvent(const UObject* WorldContext, EKRGameEvent Type, FName Name = NAME_None, float Value = 0.f, int32 Flags = 0);

	/** Which recipes random orders can use. Empty = every recipe with "In Random Pool" ticked. */
	void SetMenu(const TArray<FName>& RecipeRows);
	void AddToMenu(FName RecipeRow);

	/** How many tickets can be open at once. 0 = Max Open Tickets from DA_Tuning. */
	void SetMaxTickets(int32 Max) { MaxTicketsOverride = Max; }

	/** Put a specific ticket on the rail right now (ignores the ticket limit).
	 *  Patience: below 0 = no patience bar (never runs out); 0 = the normal formula; above 0 = that many world seconds. */
	void AddOrder(FName RecipeRow, float Patience = 0.f);

	/** Remove every ticket (no strikes). */
	void ClearOrders() { Orders.Empty(); }

	/** Tutorial mode: no strikes; a wrong dish is handed back; a ticket that runs out of patience just refills. */
	void SetTutorialMode(bool bEnabled) { bTutorialMode = bEnabled; }

	/** Is this a full night (started at Act 1)? Only full nights count for the high score. */
	void SetFullRun(bool bFull) { bFullRun = bFull; }

	/** The night is over: show the Results (won) or Game Over screen. */
	void EndShift(bool bWon, const FText& Reason, EKRRunOutcome Outcome);

	int32 GetServedCount() const { return Served; }

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

	/** Set to DA_Audio in BP_FirstPersonGameMode. Empty = no sound. */
	UPROPERTY(EditDefaultsOnly, Category = "Kitchen Rush")
	TObjectPtr<UKRAudioData> Audio;

	/** Set to DT_Recipes in BP_FirstPersonGameMode. */
	UPROPERTY(EditDefaultsOnly, Category = "Kitchen Rush")
	TObjectPtr<UDataTable> RecipeTable;

	/** Random orders on/off. (Patience keeps counting down either way. ) */
	UPROPERTY(EditAnywhere, Category = "Kitchen Rush")
	bool bOrdersEnabled = true;

private:
	int32 GetMaxTickets() const;
	void SpawnOrder();
	void AddStrike(const FText& Reason);

	/** How well Plate matches Recipe: correctness 0..1, perfect order / cook, and the trick shots,
	 *  Gus Assists and ground touches of its parts. */
	FKRPlateScore ScorePlate(const FKRRecipeRow& Recipe, const AKRPlate* Plate) const; // returns a struct now

	/** Next recipe from the shuffle bag: every dish comes up once before any repeats. */
	FName DrawRecipe();

	TArray<FKROrder> Orders;
	TArray<FName> RecipeBag;   // recipes left in the current shuffle
	TArray<FKRReceipt> Receipts;
	int32 NextReceiptId = 1;
	FName LastRecipe;          // so a new bag never starts with the dish we just had
	int32 NextOrderId = 1;
	float SecondsUntilNextOrder = 0.f;
	int32 Tips = 0;
	int32 Strikes = 0;
	bool bGameOver = false;

	TArray<FName> Menu;
	int32 MaxTicketsOverride = 0;
	bool bTutorialMode = false;
	bool bFullRun = false;
	int32 Served = 0;

	FText Message;
	double MessageExpiresAt = 0.0; // real time, so messages fade even while frozen
};
