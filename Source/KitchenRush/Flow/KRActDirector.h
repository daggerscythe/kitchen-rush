// KRActDirector.h
// [M4] Runs one Friday night: the main menu, Act 1 (Nana's tutorial), Act 2 (the dinner rush),
// Act 3 (closing time) and the end at 10:00 PM. Place one BP_ActDirector in Lvl_Kitchen.
//
// It's a "state machine": Beat says which step of the night we're on, and each step waits for
// something (the player stepping, a dish served, a catch, a time on the clock) before moving on.
// Testing console commands: kr.Act 3 (restart at an act), kr.Clock 20.4 (jump the clock to 8:24 PM).
//
// It doesn't do the work itself; it tells the GameMode (orders), Gus (tosses, trips) and the
// player controller (menus, Nana's note cards) what to do, and listens to the GameMode's events.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flow/KRFlowTypes.h"
#include "KRActDirector.generated.h"

class AKRFood;
class AKRItem;
class AKRGusDirector;
class AKitchenRushGameMode;
class AKitchenRushPlayerController;

/** Where we are in the night. */
UENUM(BlueprintType)
enum class EKRBeat : uint8
{
	None,
	MainMenu,
	Act1_Step,          // "Take a step."
	Act1_TomatoSalad,   // serve a Tomato Salad (no patience)
	Act1_Steak,         // serve a steak (generous patience)
	Act1_GardenSalad,   // serve a Garden Salad (normal patience)
	Act2_CatchOnion,    // catch Gus's onion
	Act2_ThrowSteak,    // throw a steak into the pan
	Act2_Rush,          // 5:00-7:30 PM
	Act3_Rush,          // 7:30-10:00 PM
	Done
};

UCLASS()
class KITCHENRUSH_API AKRActDirector : public AActor
{
	GENERATED_BODY()

public:
	AKRActDirector();

	virtual void Tick(float DeltaSeconds) override;

	/** The director in this level, if any. */
	static AKRActDirector* Find(const UObject* WorldContext);

	/** Start the night at Act 1, 2 or 3. The main menu's Play calls StartNight(1). */
	void StartNight(int32 Act);

	/** "Skip tutorial" on a note card: no more notes, gates or auto-freeze this run.
	 *  Pressed during Act 1, it jumps straight to the dinner rush. */
	void SkipTutorial();

	int32 GetCurrentAct() const { return CurrentAct; }
	bool IsTutorialSkipped() const { return bTutorialSkipped; }

	/** The in-game clock in hours (17.0 = 5:00 PM). Frozen at 5:00 PM during Act 1. */
	float GetClockHours() const;

	/** Testing (kr.Clock 20.4): jump the running clock to an hour. Beats you jump past fire right away. */
	void SetClockHours(float Hours);

	/** The act banner ("ACT 2 · THE DINNER RUSH") and the real time it appeared, for the HUD. */
	FText GetActTitle() const { return ActTitle; }
	double GetActTitleTime() const { return ActTitleTime; }

protected:
	virtual void BeginPlay() override;

	// ---------------- Level setup: set these on the director you placed in Lvl_Kitchen ----------------

	/** Act 1 opens on a tomato hanging in the air here (drop a Target Point near a crate). Optional. */
	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TObjectPtr<AActor> IntroTomatoSpot;

	/** BP_Item_Tomato. */
	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<AKRFood> IntroTomatoClass;

	/** Hidden until Act 3 (the pot: "the pot opens"). */
	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<TObjectPtr<AActor>> UnlockInAct3;

	/** Testing in the editor: 0 = show the main menu; 1-3 = jump straight into that act. Ignored in a packaged game. */
	UPROPERTY(EditAnywhere, Category = "Setup", meta = (ClampMin = "0", ClampMax = "3"))
	int32 EditorStartAct = 0;

	// ---------------- Recipes (row names in DT_Recipes) ----------------
	UPROPERTY(EditAnywhere, Category = "Recipes") FName TomatoSaladRow = TEXT("TomatoSalad");
	UPROPERTY(EditAnywhere, Category = "Recipes") FName GardenSaladRow = TEXT("GardenSalad");
	UPROPERTY(EditAnywhere, Category = "Recipes") FName SteakOnionsRow = TEXT("SteakOnions");
	UPROPERTY(EditAnywhere, Category = "Recipes") FName TomatoSoupRow = TEXT("TomatoSoup");
	UPROPERTY(EditAnywhere, Category = "Recipes") FName PlainSteakRow = TEXT("PlainSteak");

	// ---------------- Timeline (in-game hours: 18.25 = 6:15 PM) ----------------
	UPROPERTY(EditAnywhere, Category = "Timeline") float FirstTripHour = 18.25f;
	UPROPERTY(EditAnywhere, Category = "Timeline") float SecondTripHour = 19.f;
	UPROPERTY(EditAnywhere, Category = "Timeline") float Act3Hour = 19.5f;
	/** The chain reaction waits for a simmering pot between these two times. */
	UPROPERTY(EditAnywhere, Category = "Timeline") float ChainReactionHour = 20.5f;
	UPROPERTY(EditAnywhere, Category = "Timeline") float ChainReactionLatestHour = 21.f;
	UPROPERTY(EditAnywhere, Category = "Timeline") float ShelfCollapseHour = 21.25f;
	UPROPERTY(EditAnywhere, Category = "Timeline") float SpecialHour = 21.6667f;
	UPROPERTY(EditAnywhere, Category = "Timeline") float EndHour = 22.f;

	/** World seconds of patience on the tutorial steak (the first patience bar the player sees). */
	UPROPERTY(EditAnywhere, Category = "Timeline") float TutorialSteakPatience = 120.f;

	/** Chain reaction: the pot is pushed to boil over this many world seconds after Gus trips. */
	UPROPERTY(EditAnywhere, Category = "Timeline") float ChainReactionBoilSeconds = 6.f;

	/** World seconds after a trip's launch before the auto-freeze note (the "peak" of the burst). */
	UPROPERTY(EditAnywhere, Category = "Timeline") float AutoFreezeDelay = 0.4f;

	// ---------------- Gus events ----------------
	UPROPERTY(EditAnywhere, Category = "Gus") FName TripEvent = TEXT("Trip");
	UPROPERTY(EditAnywhere, Category = "Gus") FName ShelfEvent = TEXT("ShelfCollapse");
	/** Actors with this tag fall when the shelf collapses (their mesh must be Movable). Optional. */
	UPROPERTY(EditAnywhere, Category = "Gus") FName ShelfBoardTag = TEXT("ShelfBoard");

	// ---------------- Words (Nana's notes and Gus's lines) ----------------
	UPROPERTY(EditAnywhere, Category = "Words") FText NoteTitle;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteStep;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteTomatoSalad;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteSteak;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteBurnt;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteGardenSalad;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteHeadsUp;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteThrowSteak;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteBreathe;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NotePotOpens;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteChainReaction;
	UPROPERTY(EditAnywhere, Category = "Words", meta = (MultiLine = true)) FText NoteShelf;
	UPROPERTY(EditAnywhere, Category = "Words") FText GusDoorsOpen;
	UPROPERTY(EditAnywhere, Category = "Words") FText GusSpecial;
	UPROPERTY(EditAnywhere, Category = "Words") FText WinReason;

private:
	void BeginNight();               // the frame after BeginPlay: main menu, or straight into an act
	void StartAct(int32 Act);
	void BeginRush2();               // the clock starts
	void UpdateAct1();
	void UpdateAct2(float DeltaSeconds);
	void UpdateAct3();
	void HandleEvent(const FKRGameEvent& Event);
	void HandleGusLaunched(FName EventName);

	/** Show one of Nana's note cards (does nothing once the tutorial is skipped). */
	void Note(const FText& Body);

	/** Play Gus's trip; FreezeNote (if not empty) appears as an auto-freeze just after the launch. */
	void PlayScriptedTrip(const FText& FreezeNote);
	void TossOnion();
	void SetLocked(AActor* Actor, bool bLocked);
	void DropShelfBoards();
	void ShowActTitle(const FText& Title);
	float HoursToShiftSeconds(float Hours) const;

	/** World seconds until the next scripted trip-like beat (so Gus's timed trips stay clear of it). */
	float SecondsToNextScriptedBeat() const;

	AKitchenRushGameMode* GetGameMode() const;
	AKitchenRushPlayerController* GetPC() const;

	EKRBeat Beat = EKRBeat::None;
	int32 CurrentAct = 0;
	bool bTutorialSkipped = false;
	bool bClockRunning = false;
	float ShiftSeconds = 0.f;   // world seconds since 5:00 PM

	FVector StepStart = FVector::ZeroVector;
	TWeakObjectPtr<AKRItem> TutorialOnion;
	float OnionGoneSeconds = 0.f;

	bool bShownBurnt = false;
	bool bTrip1Done = false;
	bool bTrip2Done = false;
	bool bChainDone = false;
	bool bShelfDone = false;
	bool bSpecialDone = false;

	// Auto-freeze: after Gus launches AwaitedEvent, wait FreezeCountdown world seconds, then show FreezeNote.
	FName AwaitedEvent;
	FText FreezeNote;
	float FreezeCountdown = -1.f;

	TWeakObjectPtr<AKRGusDirector> Gus;
	FText ActTitle;
	double ActTitleTime = -100.0;
};
