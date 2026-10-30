// KRFlowTypes.h
// [M4] Small shared types for the night's flow: game events (what just happened), run outcomes and results.

#pragma once

#include "CoreMinimal.h"

/** Things that happen during a run. The GameMode broadcasts them; the act director and the playtest logger listen. */
enum class EKRGameEvent : uint8
{
	DishServed,      // Name = recipe row, Value = correctness 0..1
	DishRejected,    // tutorial only: a wrong dish handed back. Name = recipe row
	Strike,
	Catch,           // Value = 1 if Gus threw it
	TrickShot,       // Flags = KRTrick place bit (board, plate, pan, pot)
	SteakBurnt,
	KnifeHit,
	ActStarted,      // Value = act number
	TutorialSkipped,
	RunEnded         // Flags = EKRRunOutcome
};

/** How a run ended. */
enum class EKRRunOutcome : uint8
{
	Won,      // reached 10:00 PM
	Health,   // health hit 0
	Strikes,  // 3 strikes
	Quit      // left before the end (quit, restart, main menu)
};

/** One game event. */
struct FKRGameEvent
{
	EKRGameEvent Type = EKRGameEvent::DishServed;
	FName Name;
	float Value = 0.f;
	int32 Flags = 0;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FKROnGameEvent, const FKRGameEvent&);

/** What the Results / Game Over screen shows. */
struct FKRResults
{
	bool bWon = false;
	FText Reason;       // why it ended ("You're too hurt to keep cooking.")
	int32 Tips = 0;
	int32 Stars = 0;    // 0..3 (0 when you lose)
	int32 Best = 0;     // high score after this run
	bool bNewBest = false;
	int32 Served = 0;
	int32 Strikes = 0;
};
