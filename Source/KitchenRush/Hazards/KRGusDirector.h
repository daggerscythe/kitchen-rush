// KRGusDirector.h
// [M3] Gus, the clumsy waiter, as an invisible point just inside the pass window.
// He does two kinds of things:
//  - Toss: lobs you one ingredient an open ticket needs (catch it for a bonus).
//  - Events: named, data-driven bursts of flying stuff. "Trip" ships with M3; M4 adds more
//    (like the shelf collapse) and triggers them at set times instead of on a timer.
// Every launch is a list entry (what, aimed how, how long after the event starts), so new
// events need no code: add a row in BP_GusDirector's Events list.
//
// [M3.5] Everything Gus does is announced first: a big HUD warning (and an arrow if he's behind you)
// shows for a couple of world seconds BEFORE anything flies, so you have time to turn around.
//
// Debug console commands (press ` in Play): kr.Toss, kr.Trip, kr.Event <Name>, kr.Hurt <amount>, kr.GusAuto 0/1

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KRGusDirector.generated.h"

class AKRItem;
class AKRFood;
class UArrowComponent;

/** Where Gus aims one launched item. */
UENUM(BlueprintType)
enum class EKRGusAim : uint8
{
	/** Straight at your chest. For knives: step out of the way. */
	AtPlayer,
	/** Arrives at chest height a little in front of you. For things to catch. */
	InFrontOfPlayer,
	/** Lands on the floor somewhere near you. */
	NearPlayer
};

/** One thing Gus sends flying. */
USTRUCT(BlueprintType)
struct FKRGusLaunch
{
	GENERATED_BODY()

	/** BP_Knife, BP_HotSauce, BP_Item_Steak... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gus")
	TSubclassOf<AKRItem> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gus")
	EKRGusAim Aim = EKRGusAim::InFrontOfPlayer;

	/** World seconds after the event starts. Stagger launches so things don't collide at the start. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gus", meta = (ClampMin = "0"))
	float Delay = 0.f;
};

/** A named burst of launches, like "Trip". */
USTRUCT(BlueprintType)
struct FKRGusEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gus")
	FName Name;

	/** Shown big on the HUD as the warning, before anything flies. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gus")
	FText Shout;

	/** [M3.5] World seconds of warning before the first launch. 0 = use Trip Warning Seconds from DA_Tuning. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gus", meta = (ClampMin = "0"))
	float WarningSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gus")
	TArray<FKRGusLaunch> Launches;
};

UCLASS()
class KITCHENRUSH_API AKRGusDirector : public AActor
{
	GENERATED_BODY()

public:
	AKRGusDirector();

	/** Gus tosses one ingredient that an open ticket needs (or any ingredient if there are no tickets).
	 *  [M3.5] Warns first; the ingredient flies TossWarningSeconds later. */
	UFUNCTION(BlueprintCallable, Category = "Gus")
	void TossIngredient();

	/** Run the event called EventName from the Events list. False if there isn't one.
	 *  [M3.5] Warns first; the launches start after the event's warning time. Replaces a pending toss. */
	UFUNCTION(BlueprintCallable, Category = "Gus")
	bool PlayEvent(FName EventName);

	/** Spawn ItemClass at Gus and launch it. Returns the item (or nullptr). */
	AKRItem* LaunchItem(TSubclassOf<AKRItem> ItemClass, EKRGusAim Aim);

	/** Turn the automatic toss/trip timers on or off (M4 turns them off and scripts events instead). */
	UFUNCTION(BlueprintCallable, Category = "Gus")
	void SetAutoEvents(bool bEnabled) { bAutoEvents = bEnabled; }

	/** [M3.5] For the HUD: is a big trip warning up right now, and what does it say?
	 *  [M3.6] Only trips (events) use it; tosses are a normal message. */
	bool IsWarning() const { return (Pending == EPending::Event) || WarningHoldLeft > 0.f; }
	FText GetWarningText() const { return WarningText; }

	/** The Gus in this level, if any. */
	static AKRGusDirector* Find(UWorld* World);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Gus")
	TObjectPtr<USceneComponent> Root;

	/** Editor-only arrow so you can see Gus in the level. Point it into the kitchen. */
	UPROPERTY(VisibleAnywhere, Category = "Gus")
	TObjectPtr<UArrowComponent> Arrow;

	/** The ingredients Gus can toss: BP_Item_Tomato, Lettuce, Onion, Steak. */
	UPROPERTY(EditAnywhere, Category = "Gus")
	TArray<TSubclassOf<AKRFood>> FoodClasses;

	/** Named bursts. "Trip" comes with 5 rows already aimed; you only pick each row's Item Class. */
	UPROPERTY(EditAnywhere, Category = "Gus")
	TArray<FKRGusEvent> Events;

	/** Toss every TossInterval and trip every TripInterval (both in DA_Tuning). */
	UPROPERTY(EditAnywhere, Category = "Gus")
	bool bAutoEvents = true;

	/** Launches start up to this far (cm) from Gus, so a burst doesn't start inside itself. */
	UPROPERTY(EditAnywhere, Category = "Gus|Aim")
	float SpawnSpread = 25.f;

	/** InFrontOfPlayer: how far in front of you (toward Gus) catchable things arrive, cm. */
	UPROPERTY(EditAnywhere, Category = "Gus|Aim")
	float CatchPointDistance = 110.f;

	/** NearPlayer: how far from you things can land, cm. */
	UPROPERTY(EditAnywhere, Category = "Gus|Aim")
	float ScatterRadius = 200.f;

private:
	FVector GetAimPoint(EKRGusAim Aim) const;
	void Shout(const FText& Text) const;

	// [M3.5] The one thing Gus is about to do once the warning runs out.
	enum class EPending : uint8 { None, Toss, Event };
	void StartWarning(const FText& Text, float Seconds);
	void RunPending();
	void LaunchEventRows(const FKRGusEvent& Event);

	EPending Pending = EPending::None;
	TSubclassOf<AKRFood> PendingFood;
	FName PendingEvent;
	float PendingSecondsLeft = 0.f;
	float WarningHoldLeft = 0.f; // keeps the warning up a moment after things start flying
	FText WarningText;

	float SecondsToToss = 0.f;
	float SecondsToTrip = 0.f;
	bool bWarnedEmptyClass = false;
};
