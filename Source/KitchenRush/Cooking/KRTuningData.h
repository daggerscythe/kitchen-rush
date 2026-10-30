// KRTuningData.h
// Every gameplay number except the time rule

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KRTuningData.generated.h"

UCLASS(BlueprintType)
class KITCHENRUSH_API UKRTuningData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** The tuning in use: DA_Tuning from the GameMode, or these C++ defaults if none is set. */
	static const UKRTuningData* Get(const UObject* WorldContext);

	// ---- Chopping ----
	UPROPERTY(EditAnywhere, Category = "Chopping", meta = (ClampMin = "1"))
	int32 ChopsNeeded = 4;

	// ---- Steak (world seconds in the pan) ----
	UPROPERTY(EditAnywhere, Category = "Steak")
	float SteakCookedAt = 8.f;

	UPROPERTY(EditAnywhere, Category = "Steak")
	float SteakPerfectUntil = 11.f;

	UPROPERTY(EditAnywhere, Category = "Steak")
	float SteakBurntAt = 14.f;

	// ---- Soup (world seconds simmering once all tomatoes are in) ----
	UPROPERTY(EditAnywhere, Category = "Soup", meta = (ClampMin = "1"))
	int32 SoupTomatoesNeeded = 2;

	UPROPERTY(EditAnywhere, Category = "Soup")
	float SoupReadyAt = 12.f;

	/** Ladle before this for the perfect bonus; the meter turns red after it. */
	UPROPERTY(EditAnywhere, Category = "Soup")
	float SoupBoilWarnAt = 18.f;

	UPROPERTY(EditAnywhere, Category = "Soup")
	float SoupBoilOverAt = 24.f;

	// ---- Stove ----
	/** How close (cm, sideways) a pan or pot must be to a burner marker to cook. */
	UPROPERTY(EditAnywhere, Category = "Stove")
	float BurnerRadius = 25.f;

	/** How close (cm, up/down) a pan or pot must be to a burner marker to cook. */
	UPROPERTY(EditAnywhere, Category = "Stove")
	float BurnerHeight = 30.f;

	// ---- Plates ----
	UPROPERTY(EditAnywhere, Category = "Plates", meta = (ClampMin = "1"))
	int32 MaxPlateItems = 3;

	// ---- Orders (world seconds) ----
	UPROPERTY(EditAnywhere, Category = "Orders")
	float FirstOrderDelay = 3.f;

	UPROPERTY(EditAnywhere, Category = "Orders")
	float NewOrderInterval = 25.f;

	UPROPERTY(EditAnywhere, Category = "Orders", meta = (ClampMin = "1"))
	int32 MaxOpenTickets = 2;

	UPROPERTY(EditAnywhere, Category = "Orders")
	float PatienceBase = 20.f; // [M4.1] was 30: you can stop time, so tickets can be tighter

	UPROPERTY(EditAnywhere, Category = "Orders")
	float PatiencePerStep = 7.f; // [M4.1] was 10

	// ---- Scoring (dollars) ----
	/** Price of a dish whose Price in DT_Recipes is 0. */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float TipBase = 10.f;

	/** "Perfect cook!": every cooked part pulled in its perfect window. [M3.7] was 5 */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float PerfectBonus = 2.f;

	// [M3.7] SpeedBonusMax (up to +$5 scaled by patience) was replaced by the flat Patience Bonus below.

	/** [M3.7] "Patience bonus!": paid when the customer's patience bar is at least this full when served (0.7 = 70%). */
	UPROPERTY(EditAnywhere, Category = "Scoring", meta = (ClampMin = "0", ClampMax = "1"))
	float PatienceBonusThreshold = 0.7f;

	/** [M3.7] Dollars for the patience bonus. */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float PatienceBonus = 3.f;

	/** [M3.7] "Perfect order!": every part right and nothing extra. */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float PerfectOrderBonus = 1.f;

	/** [M3.7] Taken off per served ingredient that touched the floor. */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float GroundPenalty = 3.f;

	/** Correctness lost per wrong or extra item (0.25 = 25%). */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float WrongItemPenalty = 0.25f;

	/** Below this correctness, the dish earns nothing and costs a strike. */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float BadOrderThreshold = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Scoring", meta = (ClampMin = "1"))
	int32 MaxStrikes = 3;

	/** [M4] Stars on the Results screen: surviving the night = 1 star; these tips add the 2nd and 3rd.
	 *  Placeholders: the plan sets the real values after playtest round 2. */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	int32 TwoStarTips = 120;

	UPROPERTY(EditAnywhere, Category = "Scoring")
	int32 ThreeStarTips = 220;

	// ---- Shift clock ----
	UPROPERTY(EditAnywhere, Category = "Shift Clock")
	float ShiftStartHour = 17.f;

	UPROPERTY(EditAnywhere, Category = "Shift Clock")
	float WorldSecondsPerGameHour = 60.f;

	// ================= [M3] =================

	// ---- Throwing and catching ----
	/** Speed of a throw along your view, in cm/s. */
	UPROPERTY(EditAnywhere, Category = "Throwing")
	float ThrowSpeed = 900.f;

	/** Extra upward speed added to every throw, so it arcs. */
	UPROPERTY(EditAnywhere, Category = "Throwing")
	float ThrowUpSpeed = 150.f;

	/** How far away (cm) you can catch something flying. */
	UPROPERTY(EditAnywhere, Category = "Throwing")
	float CatchRange = 300.f;

	/** Catch aim assist: how close (cm) your crosshair line has to pass to a flying item. */
	UPROPERTY(EditAnywhere, Category = "Throwing")
	float CatchAssistRadius = 20.f;

	/** How hard a thrown item knocks a pan or pot off the burner (cm/s added). */
	UPROPERTY(EditAnywhere, Category = "Throwing")
	float KnockSpeed = 350.f;

	// ---- Health ----
	UPROPERTY(EditAnywhere, Category = "Health")
	float MaxHealth = 100.f;

	/** World seconds without damage before health starts coming back. */
	UPROPERTY(EditAnywhere, Category = "Health")
	float RegenDelay = 4.f;

	/** Health per world second while regenerating. */
	UPROPERTY(EditAnywhere, Category = "Health")
	float RegenPerSecond = 5.f;

	// ---- Knives ----
	UPROPERTY(EditAnywhere, Category = "Hazards|Knife")
	float KnifeHitDamage = 40.f; // [M3.6] 20 in M3, 60 in M3.5

	/** Trying to catch a knife hurts. */
	UPROPERTY(EditAnywhere, Category = "Hazards|Knife")
	float KnifeCatchDamage = 20.f; // [M3.6] 10 in M3, 30 in M3.5

	UPROPERTY(EditAnywhere, Category = "Hazards|Knife")
	float KnifeSpeed = 1100.f;

	/** 1 = normal gravity. Low values make knives fly in readable, almost straight lines. */
	UPROPERTY(EditAnywhere, Category = "Hazards|Knife")
	float KnifeGravityScale = 0.15f;

	/** A knife passing this close (cm) without hitting you earns the dodge bonus. */
	UPROPERTY(EditAnywhere, Category = "Hazards|Knife")
	float DodgeDistance = 100.f;

	// ---- Hot sauce ----
	UPROPERTY(EditAnywhere, Category = "Hazards|Hot Sauce")
	float SauceShatterDamage = 20.f; // [M3.6] 10 in M3, 30 in M3.5

	UPROPERTY(EditAnywhere, Category = "Hazards|Hot Sauce")
	float SauceShatterRadius = 150.f;

	// ---- Grease fire (pan) ----
	/** World seconds past "burnt" before the steak catches fire. */
	UPROPERTY(EditAnywhere, Category = "Hazards|Grease Fire")
	float GreaseFireDelay = 3.f;

	UPROPERTY(EditAnywhere, Category = "Hazards|Grease Fire")
	float GreaseFireSeconds = 10.f;

	UPROPERTY(EditAnywhere, Category = "Hazards|Grease Fire")
	float GreaseFireDamagePerSecond = 30.f; // [M3.6] 15 in M3, 45 in M3.5

	UPROPERTY(EditAnywhere, Category = "Hazards|Grease Fire")
	float GreaseFireRadius = 100.f;

	// ---- Boil-over (pot) ----
	UPROPERTY(EditAnywhere, Category = "Hazards|Boil Over")
	float SpillSeconds = 8.f;

	UPROPERTY(EditAnywhere, Category = "Hazards|Boil Over")
	float SpillDamagePerSecond = 20.f; // [M3.6] 10 in M3, 30 in M3.5

	UPROPERTY(EditAnywhere, Category = "Hazards|Boil Over")
	float SpillRadius = 120.f;

	// ---- Gus ----
	/** World seconds between Gus tossing you an ingredient. 0 = never (M4 schedules them instead). */
	UPROPERTY(EditAnywhere, Category = "Gus")
	float TossInterval = 40.f;

	/** World seconds between Gus tripping. 0 = never. */
	UPROPERTY(EditAnywhere, Category = "Gus")
	float TripInterval = 45.f; // [M3.5] was 90

	/** [M3.5] World seconds before Gus's first trip of the night (then every TripInterval). */
	UPROPERTY(EditAnywhere, Category = "Gus")
	float FirstTripDelay = 30.f;

	/** [M3.5] World seconds of warning before a trip: the big HUD warning and arrow show this long before anything flies.
	 *  (After launch the warning stays up 0.75 s more.) */
	UPROPERTY(EditAnywhere, Category = "Gus")
	float TripWarningSeconds = 1.2f; // [M4.1] was 1.5 (2.5 before M3.6)

	/** [M3.5] World seconds between Gus calling out a toss (a normal HUD message) and the throw. */
	UPROPERTY(EditAnywhere, Category = "Gus")
	float TossWarningSeconds = 1.5f;

	/** 0..1: how high Gus lobs things (0.5 = a nice arc). */
	UPROPERTY(EditAnywhere, Category = "Gus")
	float TossArc = 0.5f;

	// ---- Scoring extras ----
	/** [M3.7] "Gus Assist!": per ingredient caught from Gus, served, that never touched the ground. (Was CatchBonus.) */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float GusAssistBonus = 2.f;

	UPROPERTY(EditAnywhere, Category = "Scoring")
	float DodgeBonus = 1.f;

	/** Per trick shot: [M3.7] each ingredient counts once per place it was thrown into (board, plate, pan, pot),
	 *  plus one when the plate itself is thrown through the pass. */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float TrickShotBonus = 1.f;

	// ---- [M3.5] Broken plates ----
	/** A plate or bowl hitting the floor faster than this (cm/s) shatters. Dropping one off the counter is about 420. */
	UPROPERTY(EditAnywhere, Category = "Hazards|Broken Plates")
	float PlateShatterSpeed = 250.f;

	/** Damage each time you step onto a pile of shards. */
	UPROPERTY(EditAnywhere, Category = "Hazards|Broken Plates")
	float ShardStepDamage = 10.f; // [M3.6] was 15

	/** How close (cm, sideways) your feet must be to a pile of shards to step on it. */
	UPROPERTY(EditAnywhere, Category = "Hazards|Broken Plates")
	float ShardRadius = 35.f;
};
