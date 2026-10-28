// KRTimeSettings.h
// Found in Edit > Project Settings > Game > Kitchen Rush Time
// Changes are saved to Config/DefaultGame.ini

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KRTimeSettings.generated.h"

/** What world time does while the player stands still */
UENUM(BlueprintType)
enum class EKRStillMode : uint8
{
	Freeze UMETA(DisplayName = "Freeze (Option A)"),
	Slow   UMETA(DisplayName = "Slow (Option B)")
};

/** What world time does while the player walks */
UENUM(BlueprintType)
enum class EKRMoveMode : uint8
{
	Clamped  UMETA(DisplayName = "Clamped (Option A)"),
	Momentum UMETA(DisplayName = "Momentum (Option B)")
};

// Config = Game -> values are stored in the Game config file
// DefaultConfig -> edits in Project Settings go to Config/DefaultGame.ini
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Kitchen Rush Time"))
class KITCHENRUSH_API UKRTimeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Puts this page under the "Game" heading in Project Settings */
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	// ---- A/B modes (the subsystem copies these when a level starts) ----

	UPROPERTY(Config, EditAnywhere, Category = "Modes")
	EKRStillMode StillMode = EKRStillMode::Freeze;

	UPROPERTY(Config, EditAnywhere, Category = "Modes")
	EKRMoveMode MoveMode = EKRMoveMode::Clamped;

	// ---- Time scales (1.0 = normal speed) ----

	/** Standing still in Freeze mode. 0.0001 is the smallest value Unreal allows */
	UPROPERTY(Config, EditAnywhere, Category = "Scales", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float FreezeScale = 0.0001f;

	/** Standing still in Slow mode */
	UPROPERTY(Config, EditAnywhere, Category = "Scales", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float SlowScale = 0.05f;

	/** Walking (and the floor for Momentum mode) */
	UPROPERTY(Config, EditAnywhere, Category = "Scales", meta = (ClampMin = "0.1", ClampMax = "5"))
	float WalkScale = 1.0f;

	/** Momentum mode: the speed reached after walking nonstop for MomentumRampSeconds */
	UPROPERTY(Config, EditAnywhere, Category = "Scales", meta = (ClampMin = "1", ClampMax = "5"))
	float MomentumMaxScale = 1.75f;

	UPROPERTY(Config, EditAnywhere, Category = "Scales", meta = (ClampMin = "0.1"))
	float MomentumRampSeconds = 2.0f;

	// ---- Feel ----

	/** How long (real seconds) it takes to blend from one time scale to another */
	UPROPERTY(Config, EditAnywhere, Category = "Feel", meta = (ClampMin = "0"))
	float EaseSeconds = 0.15f;

	/** Chopping, picking up, placing, etc. run time at walk speed for this many real seconds */
	UPROPERTY(Config, EditAnywhere, Category = "Feel", meta = (ClampMin = "0"))
	float ActionBurstSeconds = 0.3f;

	/** The player counts as "moving" above this speed, in cm/s */
	UPROPERTY(Config, EditAnywhere, Category = "Feel", meta = (ClampMin = "0"))
	float MovingSpeedThreshold = 20.f;

	// ---- Debug ----

	/** Prints the current time scale in the top-left corner while playing */
	UPROPERTY(Config, EditAnywhere, Category = "Debug")
	bool bShowDebugText = true;
};
