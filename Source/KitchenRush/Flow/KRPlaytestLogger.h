// KRPlaytestLogger.h
// [M4] Writes one line per run to Saved/Playtests/KitchenRush_Playtests.csv, so playtests have numbers:
// real and world seconds per act, deaths, strikes, burnt steaks, catches, knife hits, tips, and which
// time modes the tester used. Open the CSV in Excel / Google Sheets.
// A world subsystem lives exactly as long as the level, so a run that's abandoned (quit, restart,
// main menu) still gets written, as "Quit", when the level closes.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Flow/KRFlowTypes.h"
#include "KRPlaytestLogger.generated.h"

UCLASS()
class KITCHENRUSH_API UKRPlaytestLogger : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void HandleEvent(const FKRGameEvent& Event);
	void CloseAct();
	void WriteRow();

	bool bRunStarted = false;
	bool bWritten = false;
	int32 StartAct = 0;
	int32 CurrentAct = 0;
	bool bTutorialSkipped = false;
	EKRRunOutcome Outcome = EKRRunOutcome::Quit;

	// Per act (index 1..3): real seconds (wall clock, includes notes and menus) and world seconds.
	double ActRealStart = 0.0;
	double ActWorldStart = 0.0;
	double RealSeconds[4] = { 0, 0, 0, 0 };
	double WorldSeconds[4] = { 0, 0, 0, 0 };
	double RunRealStart = 0.0;

	int32 Strikes = 0;
	int32 BurntSteaks = 0;
	int32 Catches = 0;
	int32 KnifeHits = 0;
	int32 TrickShots = 0;
	int32 Served = 0;
	int32 FinalTips = 0;
};
