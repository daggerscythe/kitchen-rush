// KRAudioData.h
// DA_Audio: which file plays for every sound, how loud, and how time bends them.
// Make it once: Content Browser > right-click > Miscellaneous > Data Asset > KRAudioData, name it DA_Audio,
// then set it as "Audio" on BP_FirstPersonGameMode. Any slot left empty is simply silent.
//
// Tip: you can change volumes WHILE playing (PIE). Every sound reads its volume from here as it plays.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Audio/KRAudioTypes.h"
#include "KRAudioData.generated.h"

class USoundBase;

/** One sound slot: the file(s), how loud, and whether time bends it. */
USTRUCT(BlueprintType)
struct FKRSound
{
	GENERATED_BODY()

	/** The file to play (a Sound Wave or Sound Cue). Empty = this sound is silent. */
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> Sound;

	/** Optional extra versions (Chop_2, Chop_3...). Each play picks one of Sound + these at random. */
	UPROPERTY(EditAnywhere, Category = "Sound")
	TArray<TObjectPtr<USoundBase>> Variations;

	/** 1 = the file as it is. 0.5 = half as loud, 2 = twice as loud. */
	UPROPERTY(EditAnywhere, Category = "Sound", meta = (ClampMin = "0", UIMax = "4"))
	float Volume = 1.f;

	/** Random pitch change each play (0.05 = up to 5% higher or lower), so repeats don't sound robotic. */
	UPROPERTY(EditAnywhere, Category = "Sound", meta = (ClampMin = "0", ClampMax = "0.5"))
	float PitchJitter = 0.f;

	/** Pitched down and muffled while time is slowed. On for world sounds, off for the player's own and the UI. */
	UPROPERTY(EditAnywhere, Category = "Sound")
	bool bBendsWithTime = false;

	FKRSound() {}
	FKRSound(float InVolume, bool bInBends, float InJitter = 0.f)
		: Volume(InVolume), PitchJitter(InJitter), bBendsWithTime(bInBends) {}

	/** Sound or one of the Variations at random; nullptr if none are set. */
	USoundBase* Pick() const;
};

UCLASS(BlueprintType)
class KITCHENRUSH_API UKRAudioData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** DA_Audio from the GameMode, or nullptr if it isn't set (then the game is silent). */
	static const UKRAudioData* Get(const UObject* WorldContext);

	/** The slot for a sound name, or nullptr for EKRSound::None. */
	const FKRSound* Find(EKRSound Id) const;

	// The starting volumes below were set from how loud each of your files is,
	// so everything starts roughly level. Fine-tune by ear.

	// ---- Time and atmosphere ----
	UPROPERTY(EditAnywhere, Category = "Time and Atmosphere") FKRSound WatchTick = FKRSound(1.0f, false);
	UPROPERTY(EditAnywhere, Category = "Time and Atmosphere") FKRSound WatchTock = FKRSound(0.9f, false);
	UPROPERTY(EditAnywhere, Category = "Time and Atmosphere") FKRSound TimeSlow = FKRSound(0.6f, false);
	UPROPERTY(EditAnywhere, Category = "Time and Atmosphere") FKRSound TimeResume = FKRSound(0.5f, false);
	UPROPERTY(EditAnywhere, Category = "Time and Atmosphere") FKRSound DinerMurmur = FKRSound(1.2f, true);

	// ---- Music and stings ----
	/** Muffled (never pitched) while time is slowed: see Music Frozen Volume / Low Pass below. */
	UPROPERTY(EditAnywhere, Category = "Music and Stings") FKRSound Jukebox = FKRSound(0.4f, false);
	UPROPERTY(EditAnywhere, Category = "Music and Stings") FKRSound Win = FKRSound(0.6f, false);
	UPROPERTY(EditAnywhere, Category = "Music and Stings") FKRSound GameOver = FKRSound(0.8f, false);

	// ---- Player ----
	UPROPERTY(EditAnywhere, Category = "Player") FKRSound Footstep = FKRSound(0.2f, false, 0.08f);
	UPROPERTY(EditAnywhere, Category = "Player") FKRSound Throw = FKRSound(0.5f, false, 0.06f);
	UPROPERTY(EditAnywhere, Category = "Player") FKRSound Hurt = FKRSound(1.0f, false, 0.05f);
	UPROPERTY(EditAnywhere, Category = "Player") FKRSound Heartbeat = FKRSound(0.3f, false);

	// ---- Cooking ----
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound Chop = FKRSound(3.5f, false, 0.06f);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound Sizzle = FKRSound(0.9f, true);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound CookDone = FKRSound(0.4f, true);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound FireFlare = FKRSound(0.4f, true);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound FireLoop = FKRSound(0.35f, true);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound PotBubble = FKRSound(0.25f, true);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound Plop = FKRSound(2.5f, true, 0.08f);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound BoilOver = FKRSound(0.7f, true);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound DishClink = FKRSound(0.3f, true, 0.06f);
	UPROPERTY(EditAnywhere, Category = "Cooking") FKRSound TrashDump = FKRSound(0.35f, true, 0.05f);

	// ---- Throws landing ----
	UPROPERTY(EditAnywhere, Category = "Throws Landing") FKRSound Thud = FKRSound(0.75f, true, 0.1f);
	UPROPERTY(EditAnywhere, Category = "Throws Landing") FKRSound PanClang = FKRSound(0.45f, true, 0.05f);
	UPROPERTY(EditAnywhere, Category = "Throws Landing") FKRSound PotClang = FKRSound(0.4f, true, 0.05f);

	// ---- Gus and hazards ----
	/** Gus's voice stays clear (not bent) so the warning is always understood. Tick Bends With Time to change that. */
	UPROPERTY(EditAnywhere, Category = "Gus and Hazards") FKRSound GusHeadsUp = FKRSound(0.3f, false);
	UPROPERTY(EditAnywhere, Category = "Gus and Hazards") FKRSound GusWatchOut = FKRSound(0.45f, false);
	UPROPERTY(EditAnywhere, Category = "Gus and Hazards") FKRSound KnifeWhoosh = FKRSound(0.9f, true);
	UPROPERTY(EditAnywhere, Category = "Gus and Hazards") FKRSound Shatter = FKRSound(0.4f, true, 0.05f);
	UPROPERTY(EditAnywhere, Category = "Gus and Hazards") FKRSound ShardCrunch = FKRSound(1.5f, false, 0.08f);
	UPROPERTY(EditAnywhere, Category = "Gus and Hazards") FKRSound Splat = FKRSound(0.5f, true, 0.05f);
	UPROPERTY(EditAnywhere, Category = "Gus and Hazards") FKRSound ShelfCreak = FKRSound(1.8f, true);
	UPROPERTY(EditAnywhere, Category = "Gus and Hazards") FKRSound ShelfCrash = FKRSound(0.65f, true);

	// ---- Orders and UI ----
	UPROPERTY(EditAnywhere, Category = "Orders and UI") FKRSound NewOrder = FKRSound(0.85f, true);
	UPROPERTY(EditAnywhere, Category = "Orders and UI") FKRSound Served = FKRSound(1.6f, false);
	UPROPERTY(EditAnywhere, Category = "Orders and UI") FKRSound NoteRustle = FKRSound(2.5f, false, 0.05f);
	UPROPERTY(EditAnywhere, Category = "Orders and UI") FKRSound UIClick = FKRSound(2.3f, false, 0.03f);

	// ---- How time bends sound ----

	/** World sounds' pitch when time is fully slowed (1 = no change). Unreal won't go below about 0.4. */
	UPROPERTY(EditAnywhere, Category = "Time Bend", meta = (ClampMin = "0.4", ClampMax = "1"))
	float FrozenPitch = 0.5f;

	/** World sounds are muffled down to this many Hz when time is fully slowed (20000 = not muffled). */
	UPROPERTY(EditAnywhere, Category = "Time Bend", meta = (ClampMin = "100", ClampMax = "20000"))
	float FrozenLowPassHz = 1000.f;

	/** The jukebox's volume when time is fully slowed (1 = no change). The music never changes pitch. */
	UPROPERTY(EditAnywhere, Category = "Time Bend", meta = (ClampMin = "0", ClampMax = "1"))
	float MusicFrozenVolume = 0.45f;

	/** The jukebox is muffled down to this many Hz when time is fully slowed: "behind the wall". */
	UPROPERTY(EditAnywhere, Category = "Time Bend", meta = (ClampMin = "100", ClampMax = "20000"))
	float MusicFrozenLowPassHz = 700.f;

	/** Seconds of world time between watch ticks. */
	UPROPERTY(EditAnywhere, Category = "Time Bend", meta = (ClampMin = "0.1"))
	float TickInterval = 1.f;

	/** Real seconds that must pass between two tape-stop / spin-up sounds, so tapping W doesn't spam them. */
	UPROPERTY(EditAnywhere, Category = "Time Bend", meta = (ClampMin = "0"))
	float RampSoundCooldown = 0.5f;

	// ---- Player ----

	/** Distance walked between footsteps, in cm. */
	UPROPERTY(EditAnywhere, Category = "Player Tuning", meta = (ClampMin = "10"))
	float StepDistance = 170.f;

	/** Crouched steps are shorter and quieter. */
	UPROPERTY(EditAnywhere, Category = "Player Tuning", meta = (ClampMin = "10"))
	float CrouchStepDistance = 110.f;

	UPROPERTY(EditAnywhere, Category = "Player Tuning", meta = (ClampMin = "0", ClampMax = "1"))
	float CrouchStepVolume = 0.5f;

	/** The heartbeat starts below this much health (0.35 = 35%) and gets louder as health drops. */
	UPROPERTY(EditAnywhere, Category = "Player Tuning", meta = (ClampMin = "0", ClampMax = "1"))
	float LowHealthPercent = 0.35f;

	/** Real seconds between hurt sounds, so standing in fire doesn't grunt every frame. */
	UPROPERTY(EditAnywhere, Category = "Player Tuning", meta = (ClampMin = "0"))
	float HurtSoundInterval = 0.6f;

	// ---- 3D ----

	/** Sounds in the world are full volume within this distance (cm), then fade out... */
	UPROPERTY(EditAnywhere, Category = "3D", meta = (ClampMin = "0"))
	float FullVolumeRadius = 400.f;

	/** ...over this much more distance (cm). */
	UPROPERTY(EditAnywhere, Category = "3D", meta = (ClampMin = "1"))
	float FalloffDistance = 2500.f;
};
