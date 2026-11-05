// KRAudioSubsystem.h
// Plays every sound in the game and makes time audible:
//   - world sounds (sizzle, knives, Gus's mess) drop in pitch and get muffled as time slows
//   - Grandma's watch ticks once per second of WORLD time, so it slows and stops with time
//   - a tape-stop plays when you stop moving, a spin-up when you start
//   - the jukebox gets quieter and muffled (never pitched) while time is slowed
// It also plays the player's footsteps and the low-health heartbeat, and adds a click to every menu button.
//
// From anywhere in the game:
//     UKRAudioSubsystem::PlayAt(this, EKRSound::Chop, GetActorLocation());   // a sound at a place
//     UKRAudioSubsystem::Play2D(this, EKRSound::Throw);                      // a sound with no place
//     UKRAudioSubsystem::UpdateLoop(this, CookLoop, EKRSound::Sizzle, bCooking, Mesh); // start/stop a loop

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Audio/KRAudioTypes.h"
#include "KRAudioSubsystem.generated.h"

class APawn;
class UAudioComponent;
class USceneComponent;
class USoundAttenuation;
class UUserWidget;
class UKRAudioData;
struct FKRSound;

UCLASS()
class KITCHENRUSH_API UKRAudioSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The audio subsystem of WorldContext's world, or nullptr (e.g. in the editor, not playing). */
	static UKRAudioSubsystem* Get(const UObject* WorldContext);

	/** Play a sound once at a place in the world (you hear where it is). */
	static void PlayAt(const UObject* WorldContext, EKRSound Sound, const FVector& Location);

	/** Play a sound once with no place (the player's own sounds, UI).
	 *  bUI: keep playing while the game is paused (menus, notes, results). */
	static void Play2D(const UObject* WorldContext, EKRSound Sound, bool bUI = false, float VolumeScale = 1.f);

	/** Start or stop a looping sound. Call it every frame (or whenever things change) with whether it should play.
	 *  Loop is a variable you keep on your actor: it holds the playing sound between calls.
	 *  AttachTo: the sound follows this component (nullptr = no place). */
	static void UpdateLoop(const UObject* WorldContext, TObjectPtr<UAudioComponent>& Loop, EKRSound Sound, bool bShouldPlay, USceneComponent* AttachTo);

	/** Play a sound once at the pass window (the order bell). 2D if there's no pass in the level. */
	static void PlayAtPass(const UObject* WorldContext, EKRSound Sound);

	/** The player got hurt: a grunt, but not more often than Hurt Sound Interval (fire hurts every frame). */
	static void PlayHurt(const UObject* WorldContext);

	/** Give every button on this screen a click sound. The player controller calls it when a screen opens. */
	void AddClickSounds(UUserWidget* Screen);

	/** The night is over: fade out the jukebox and play the win or game-over jingle. */
	void PlayResultsJingle(bool bWon);

	// ---- Ticking ----
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** Keep ticking in menus, so the music can come back to full volume there. */
	virtual bool IsTickableWhenPaused() const override { return true; }

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	/** A sound we started and keep adjusting every frame (pitch, muffling, volume). */
	struct FActiveSound
	{
		TWeakObjectPtr<UAudioComponent> Component;
		EKRSound Id = EKRSound::None;
		float BasePitch = 1.f;    // includes the random jitter
		float VolumeScale = 1.f;  // e.g. quieter crouched steps, the heartbeat getting louder
		float LastVolume = -1.f;  // what we last sent, so we only send changes
		float LastPitch = -1.f;
		float LastLowPassHz = -1.f;
	};

	/** Make and start a sound. Location = 3D at that spot, AttachTo = 3D following it, neither = 2D. */
	UAudioComponent* Spawn(EKRSound Id, const FVector* Location, USceneComponent* AttachTo, bool bUI, float VolumeScale);

	/** The volume, pitch and muffling a sound should have right now. */
	void ComputeBend(const FActiveSound& Sound, const FKRSound& Slot, const UKRAudioData* Data, float& OutVolume, float& OutPitch, float& OutLowPassHz) const;

	/** Push the current pitch / muffling / volume to one sound. */
	void ApplyBend(FActiveSound& Sound, const UKRAudioData* Data, bool bWorldUpdate);

	/** 0..1: how slowed the world sounds are (and the music, which follows more gently). */
	float WorldBend = 0.f;
	float MusicBend = 0.f;

	void TickTimeSounds(const UKRAudioData* Data, APawn* Pawn);
	void TickPlayerSounds(const UKRAudioData* Data, APawn* Pawn, float RealDt);

	/** Keep a background loop running (starts it, and restarts it if the file isn't set to loop). */
	void KeepLoopRunning(TObjectPtr<UAudioComponent>& Loop, double& NextTryTime, EKRSound Id, const FVector* Location, bool bUI);

	void SetVolumeScale(UAudioComponent* Component, float Scale);

	/** Find the pass window once and remember where it is. */
	void FindPass();

	/** The 3D falloff every world sound uses (made from the 3D settings in DA_Audio). */
	USoundAttenuation* GetAttenuation(const UKRAudioData* Data);

	UFUNCTION()
	void HandleButtonClicked();

	UPROPERTY() TObjectPtr<USoundAttenuation> Attenuation;
	UPROPERTY() TObjectPtr<UAudioComponent> MusicLoop;
	UPROPERTY() TObjectPtr<UAudioComponent> MurmurLoop;
	UPROPERTY() TObjectPtr<UAudioComponent> HeartbeatLoop;

	TArray<FActiveSound> Active;

	// Volume settings from the Settings screen
	float MasterVolume = 1.f;
	float MusicVolume = 1.f;

	// The diner murmur comes from the pass
	bool bLookedForPass = false;
	bool bHasPass = false;
	FVector PassLocation = FVector::ZeroVector;

	bool bMusicStopped = false;      // after the results jingle
	double MusicNextTry = 0.0;       // KeepLoopRunning: don't retry more than once a second
	double MurmurNextTry = 0.0;

	// Watch tick
	float LastWorldSeconds = -1.f;
	float TickAccumulator = 0.f;
	bool bNextIsTock = false;

	// Tape-stop / spin-up
	bool bTimeRunning = false;
	double LastRampSoundTime = -100.0;

	// Footsteps and hurt
	float StepAccumulator = 0.f;
	double LastHurtSoundTime = -100.0;
};
