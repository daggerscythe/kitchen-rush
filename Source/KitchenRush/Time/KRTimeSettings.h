// KRTimeSettings.h
// Found in Edit > Project Settings > Game > Kitchen Rush Time
// Changes are saved to Config/DefaultGame.ini

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KRTimeSettings.generated.h"

// Config = Game -> values are stored in the Game config file
// DefaultConfig -> edits in Project Settings go to Config/DefaultGame.ini
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Kitchen Rush Time"))
class KITCHENRUSH_API UKRTimeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Puts this page under the "Game" heading in Project Settings */
	virtual FName GetCategoryName() const override { return TEXT("Game"); }



	// ---- Time scales (1.0 = normal speed) ----

	/** Standing still: the world slows to this speed (0.05 = one twentieth). */
	UPROPERTY(Config, EditAnywhere, Category = "Scales", meta = (ClampMin = "0.0001", ClampMax = "1"))
	float SlowScale = 0.05f;

	/** Walking */
	UPROPERTY(Config, EditAnywhere, Category = "Scales", meta = (ClampMin = "0.1", ClampMax = "5"))
	float WalkScale = 1.0f;

	// ---- Feel ----

	/** Real seconds to ramp from standing still to full walking speed. Slow = Superhot feel. */
	UPROPERTY(Config, EditAnywhere, Category = "Feel", meta = (ClampMin = "0.01"))
	float SpeedUpSeconds = 1.5f;

	/** Real seconds to ramp from walking back down to slow. It starts the moment you stop. */
	UPROPERTY(Config, EditAnywhere, Category = "Feel", meta = (ClampMin = "0.01"))
	float SlowDownSeconds = 1.0f;

	/** Actions (chop, pick up...) ramp time up much faster than walking does. */
	UPROPERTY(Config, EditAnywhere, Category = "Feel", meta = (ClampMin = "0.01"))
	float ActionRampSeconds = 0.08f;

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
