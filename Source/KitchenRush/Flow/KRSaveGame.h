// KRSaveGame.h
// [M4] What's saved between sessions: settings, high score and the furthest act reached (for Continue).
// A USaveGame is just a UObject whose UPROPERTYs get written to a file in Saved/SaveGames.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Time/KRTimeSettings.h"
#include "KRSaveGame.generated.h"

UCLASS()
class KITCHENRUSH_API UKRSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Best tips on a full night (started from Act 1). */
	UPROPERTY()
	int32 HighScore = 0;

	/** Furthest act started, for the main menu's Continue (1-3). */
	UPROPERTY()
	int32 LastActReached = 1;

	UPROPERTY()
	bool bHasWon = false;

	// ---- Settings ----
	UPROPERTY()
	bool bSettingsInitialised = false; // false = take the defaults from Project Settings on first run

	UPROPERTY()
	float MouseSensitivity = 1.f;

	UPROPERTY()
	float UIScale = 1.f;

	UPROPERTY()
	EKRStillMode StillMode = EKRStillMode::Slow;

	UPROPERTY()
	EKRMoveMode MoveMode = EKRMoveMode::Clamped;
};
