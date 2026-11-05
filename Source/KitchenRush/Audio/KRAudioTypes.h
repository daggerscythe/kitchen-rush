// KRAudioTypes.h
// The name of every sound in the game. Code asks for a sound by name (EKRSound::Chop);
// DA_Audio says which file plays and how loud. Plain C++ (no UENUM), so any header can include it cheaply.

#pragma once

#include "CoreMinimal.h"

enum class EKRSound : uint8
{
	None,

	// Time and atmosphere
	WatchTick,     // every other world-time second...
	WatchTock,     // ...alternating with this one
	TimeSlow,      // tape-stop when you stop moving
	TimeResume,    // spin-up when you start again
	DinerMurmur,   // dining-room loop, from the pass

	// Music and stings
	Jukebox,       // music loop (main menu and in game)
	Win,
	GameOver,

	// Player
	Footstep,
	Throw,
	Hurt,
	Heartbeat,     // loop while health is low

	// Cooking
	Chop,
	Sizzle,        // loop while a steak is in the pan on a burner
	CookDone,      // steak or soup reaches its perfect window
	FireFlare,     // a grease fire starts
	FireLoop,      // loop while the fire burns
	PotBubble,     // loop while the soup simmers
	Plop,          // tomato into the pot
	BoilOver,
	DishClink,     // plate or bowl set down; food landing on a plate
	TrashDump,

	// Throws landing
	Thud,          // food (or a knife) hitting something that doesn't catch it
	PanClang,
	PotClang,

	// Gus and hazards
	GusHeadsUp,    // before he tosses an ingredient
	GusWatchOut,   // before a trip or the shelf collapse
	KnifeWhoosh,   // loop while a knife flies
	Shatter,       // plate, bowl or hot sauce bottle breaking
	ShardCrunch,   // stepping on broken pieces
	Splat,         // hot sauce
	ShelfCreak,
	ShelfCrash,

	// Orders and UI
	NewOrder,
	Served,
	NoteRustle,
	UIClick
};
