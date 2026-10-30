// KRProfileSubsystem.h
// [M4] The player's profile: loads the save file when the game starts, keeps it while levels change,
// and saves whenever something changes. A GameInstance subsystem lives as long as the game runs
// (it survives OpenLevel), so settings and the high score are always at hand:
//     GetGameInstance()->GetSubsystem<UKRProfileSubsystem>()

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Time/KRTimeSettings.h"
#include "KRProfileSubsystem.generated.h"

class UKRSaveGame;

UCLASS()
class KITCHENRUSH_API UKRProfileSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Shortcut: the profile from anything in the world (nullptr if there's no game instance). */
	static UKRProfileSubsystem* Get(const UObject* WorldContext);

	// ---- Progress ----
	int32 GetHighScore() const;
	int32 GetLastActReached() const;

	/** Remember that the player started this act (for Continue). */
	void RecordAct(int32 Act);

	/** Record a full night's tips. Returns true if it's a new high score. */
	bool RecordTips(int32 Tips, bool bWon);

	// ---- Settings (each setter saves and applies) ----
	float GetMouseSensitivity() const;
	void SetMouseSensitivity(float Value);

	float GetUIScale() const;
	void SetUIScale(float Value);

	EKRStillMode GetStillMode() const;
	void SetStillMode(EKRStillMode Mode, UWorld* World);

	EKRMoveMode GetMoveMode() const;
	void SetMoveMode(EKRMoveMode Mode, UWorld* World);

	/** Push the saved settings into the running game (time modes, UI scale). The GameMode calls it on BeginPlay. */
	void ApplySettings(UWorld* World) const;

private:
	void Save();

	UPROPERTY()
	TObjectPtr<UKRSaveGame> Data;
};
