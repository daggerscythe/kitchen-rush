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
	float PatienceBase = 30.f;

	UPROPERTY(EditAnywhere, Category = "Orders")
	float PatiencePerStep = 10.f;

	// ---- Scoring (dollars) ----
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float TipBase = 10.f;

	UPROPERTY(EditAnywhere, Category = "Scoring")
	float PerfectBonus = 5.f;

	UPROPERTY(EditAnywhere, Category = "Scoring")
	float SpeedBonusMax = 5.f;

	/** Correctness lost per wrong or extra item (0.25 = 25%). */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float WrongItemPenalty = 0.25f;

	/** Below this correctness, the dish earns nothing and costs a strike. */
	UPROPERTY(EditAnywhere, Category = "Scoring")
	float BadOrderThreshold = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Scoring", meta = (ClampMin = "1"))
	int32 MaxStrikes = 3;

	// ---- Shift clock ----
	UPROPERTY(EditAnywhere, Category = "Shift Clock")
	float ShiftStartHour = 17.f;

	UPROPERTY(EditAnywhere, Category = "Shift Clock")
	float WorldSecondsPerGameHour = 60.f;
};
