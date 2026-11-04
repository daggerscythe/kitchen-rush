// KRAudioSubsystem.cpp

#include "Audio/KRAudioSubsystem.h"
#include "Audio/KRAudioData.h"
#include "Time/KRTimeSubsystem.h"
#include "Time/KRTimeSettings.h"
#include "Flow/KRProfileSubsystem.h"
#include "Orders/KRPass.h"
#include "Player/KRHealthComponent.h"
#include "Components/AudioComponent.h"
#include "Components/Button.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformTime.h"
#include "Misc/App.h"

// "Not muffled": 20000 Hz is the top of human hearing.
static constexpr float OpenLowPassHz = 20000.f;

/** Blend between two frequencies the way ears hear them (by octaves, not by Hz). */
static float LogLerp(float A, float B, float Alpha)
{
	return FMath::Exp(FMath::Lerp(FMath::Loge(A), FMath::Loge(B), Alpha));
}

UKRAudioSubsystem* UKRAudioSubsystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UKRAudioSubsystem>() : nullptr;
}

bool UKRAudioSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TStatId UKRAudioSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UKRAudioSubsystem, STATGROUP_Tickables);
}

// ============================================================================
// Playing sounds
// ============================================================================

void UKRAudioSubsystem::PlayAt(const UObject* WorldContext, EKRSound Sound, const FVector& Location)
{
	if (UKRAudioSubsystem* Audio = Get(WorldContext))
	{
		Audio->Spawn(Sound, &Location, nullptr, false, 1.f);
	}
}

void UKRAudioSubsystem::Play2D(const UObject* WorldContext, EKRSound Sound, bool bUI, float VolumeScale)
{
	if (UKRAudioSubsystem* Audio = Get(WorldContext))
	{
		Audio->Spawn(Sound, nullptr, nullptr, bUI, VolumeScale);
	}
}

void UKRAudioSubsystem::UpdateLoop(const UObject* WorldContext, TObjectPtr<UAudioComponent>& Loop, EKRSound Sound, bool bShouldPlay, USceneComponent* AttachTo)
{
	// A loop whose component is gone (its actor was destroyed, or the file wasn't set to loop and ended) isn't playing.
	if (Loop && !IsValid(Loop))
	{
		Loop = nullptr;
	}
	const bool bPlaying = Loop != nullptr;
	if (bShouldPlay == bPlaying)
	{
		return;
	}
	if (!bShouldPlay)
	{
		Loop->FadeOut(0.2f, 0.f); // it's set to destroy itself once it has faded out
		Loop = nullptr;
		return;
	}
	if (UKRAudioSubsystem* Audio = Get(WorldContext))
	{
		Loop = Audio->Spawn(Sound, nullptr, AttachTo, false, 1.f);
	}
}

void UKRAudioSubsystem::PlayAtPass(const UObject* WorldContext, EKRSound Sound)
{
	if (UKRAudioSubsystem* Audio = Get(WorldContext))
	{
		Audio->FindPass();
		Audio->Spawn(Sound, Audio->bHasPass ? &Audio->PassLocation : nullptr, nullptr, false, 1.f);
	}
}

void UKRAudioSubsystem::FindPass()
{
	if (bLookedForPass)
	{
		return;
	}
	bLookedForPass = true;
	for (TActorIterator<AKRPass> It(GetWorld()); It; ++It)
	{
		PassLocation = It->GetActorLocation();
		bHasPass = true;
		break;
	}
}

void UKRAudioSubsystem::PlayHurt(const UObject* WorldContext)
{
	UKRAudioSubsystem* Audio = Get(WorldContext);
	const UKRAudioData* Data = UKRAudioData::Get(WorldContext);
	if (!Audio || !Data)
	{
		return;
	}
	const double Now = FPlatformTime::Seconds();
	if (Now - Audio->LastHurtSoundTime >= Data->HurtSoundInterval)
	{
		Audio->LastHurtSoundTime = Now;
		Audio->Spawn(EKRSound::Hurt, nullptr, nullptr, false, 1.f);
	}
}

UAudioComponent* UKRAudioSubsystem::Spawn(EKRSound Id, const FVector* Location, USceneComponent* AttachTo, bool bUI, float VolumeScale)
{
	const UKRAudioData* Data = UKRAudioData::Get(this);
	const FKRSound* Slot = Data ? Data->Find(Id) : nullptr;
	USoundBase* File = Slot ? Slot->Pick() : nullptr;
	if (!File)
	{
		return nullptr; // nothing set for this sound: silent
	}

	FActiveSound Entry;
	Entry.Id = Id;
	Entry.BasePitch = 1.f + FMath::FRandRange(-Slot->PitchJitter, Slot->PitchJitter);
	Entry.VolumeScale = VolumeScale;

	// Work out the volume, pitch and muffling it should start with (time may already be slowed).
	float Volume = 1.f, Pitch = 1.f, LowPassHz = OpenLowPassHz;
	ComputeBend(Entry, *Slot, Data, Volume, Pitch, LowPassHz);

	UAudioComponent* Component = nullptr;
	if (AttachTo)
	{
		// Follows AttachTo around, and stops if its actor is destroyed.
		Component = UGameplayStatics::SpawnSoundAttached(File, AttachTo, NAME_None, FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset, true, Volume, Pitch, 0.f, GetAttenuation(Data), nullptr, true);
	}
	else if (Location)
	{
		Component = UGameplayStatics::SpawnSoundAtLocation(this, File, *Location, FRotator::ZeroRotator,
			Volume, Pitch, 0.f, GetAttenuation(Data), nullptr, true);
	}
	else
	{
		// 2D: CreateSound2D makes it without playing, so we can mark it as a UI sound first.
		Component = UGameplayStatics::CreateSound2D(this, File, Volume, Pitch, 0.f, nullptr, false, true);
		if (Component)
		{
			Component->bIsUISound = bUI; // UI sounds keep playing while the game is paused
			Component->Play();
		}
	}
	if (!Component)
	{
		return nullptr;
	}
	if (Slot->bBendsWithTime || Id == EKRSound::Jukebox)
	{
		Component->SetLowPassFilterEnabled(true);
		Component->SetLowPassFilterFrequency(LowPassHz);
	}
	Entry.Component = Component;
	Entry.LastVolume = Volume;
	Entry.LastPitch = Pitch;
	Entry.LastLowPassHz = LowPassHz;
	Active.Add(Entry);
	return Component;
}

void UKRAudioSubsystem::ComputeBend(const FActiveSound& Sound, const FKRSound& Slot, const UKRAudioData* Data, float& OutVolume, float& OutPitch, float& OutLowPassHz) const
{
	OutVolume = Slot.Volume * Sound.VolumeScale * MasterVolume;
	OutPitch = Sound.BasePitch;
	OutLowPassHz = OpenLowPassHz;
	if (Sound.Id == EKRSound::Jukebox)
	{
		// The music never changes pitch (that would put it out of tune): it just sinks behind the wall.
		OutVolume *= MusicVolume * FMath::Lerp(1.f, Data->MusicFrozenVolume, MusicBend);
		OutLowPassHz = LogLerp(OpenLowPassHz, Data->MusicFrozenLowPassHz, MusicBend);
	}
	else if (Slot.bBendsWithTime)
	{
		OutPitch *= FMath::Lerp(1.f, Data->FrozenPitch, WorldBend);
		OutLowPassHz = LogLerp(OpenLowPassHz, Data->FrozenLowPassHz, WorldBend);
	}
}

void UKRAudioSubsystem::ApplyBend(FActiveSound& Sound, const UKRAudioData* Data, bool bWorldUpdate)
{
	UAudioComponent* Component = Sound.Component.Get();
	const FKRSound* Slot = Data ? Data->Find(Sound.Id) : nullptr;
	if (!Component || !Slot)
	{
		return;
	}
	const bool bMusic = Sound.Id == EKRSound::Jukebox;
	if (!bMusic && !bWorldUpdate)
	{
		return; // world sounds are paused along with the game: leave them as they are
	}
	float Volume, Pitch, LowPassHz;
	ComputeBend(Sound, *Slot, Data, Volume, Pitch, LowPassHz);

	// Only send what changed (each call is a message to the audio thread).
	if (!FMath::IsNearlyEqual(Volume, Sound.LastVolume, 0.001f))
	{
		Component->SetVolumeMultiplier(Volume);
		Sound.LastVolume = Volume;
	}
	if (!FMath::IsNearlyEqual(Pitch, Sound.LastPitch, 0.001f))
	{
		Component->SetPitchMultiplier(Pitch);
		Sound.LastPitch = Pitch;
	}
	if ((bMusic || Slot->bBendsWithTime) && !FMath::IsNearlyEqual(LowPassHz, Sound.LastLowPassHz, 1.f))
	{
		Component->SetLowPassFilterFrequency(LowPassHz);
		Sound.LastLowPassHz = LowPassHz;
	}
}

void UKRAudioSubsystem::SetVolumeScale(UAudioComponent* Component, float Scale)
{
	for (FActiveSound& Sound : Active)
	{
		if (Sound.Component.Get() == Component)
		{
			Sound.VolumeScale = Scale;
			return;
		}
	}
}

USoundAttenuation* UKRAudioSubsystem::GetAttenuation(const UKRAudioData* Data)
{
	if (!Attenuation && Data)
	{
		// Made in code so there's no extra asset to set up. Same settings as a Sound Attenuation asset.
		Attenuation = NewObject<USoundAttenuation>(this);
		FSoundAttenuationSettings& Settings = Attenuation->Attenuation;
		Settings.bAttenuate = true;   // quieter with distance
		Settings.bSpatialize = true;  // left/right: you can tell where it is
		Settings.AttenuationShape = EAttenuationShape::Sphere;
		Settings.AttenuationShapeExtents = FVector(Data->FullVolumeRadius, 0.f, 0.f); // X = the sphere's radius
		Settings.FalloffDistance = Data->FalloffDistance;
	}
	return Attenuation;
}

// ============================================================================
// Menus and results
// ============================================================================

void UKRAudioSubsystem::AddClickSounds(UUserWidget* Screen)
{
	if (!Screen || !Screen->WidgetTree)
	{
		return;
	}
	// Every Button anywhere in the screen. AddUniqueDynamic: opening the screen again doesn't add a second click.
	Screen->WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UButton* Button = Cast<UButton>(Widget))
		{
			Button->OnClicked.AddUniqueDynamic(this, &UKRAudioSubsystem::HandleButtonClicked);
		}
	});
}

void UKRAudioSubsystem::HandleButtonClicked()
{
	Spawn(EKRSound::UIClick, nullptr, nullptr, true, 1.f);
}

void UKRAudioSubsystem::PlayResultsJingle(bool bWon)
{
	bMusicStopped = true;
	if (MusicLoop && IsValid(MusicLoop))
	{
		MusicLoop->FadeOut(1.f, 0.f);
	}
	MusicLoop = nullptr;
	if (HeartbeatLoop && IsValid(HeartbeatLoop))
	{
		HeartbeatLoop->Stop();
	}
	HeartbeatLoop = nullptr;
	Spawn(bWon ? EKRSound::Win : EKRSound::GameOver, nullptr, nullptr, true, 1.f);
}

// ============================================================================
// Every frame (real time; keeps running in menus)
// ============================================================================

void UKRAudioSubsystem::KeepLoopRunning(TObjectPtr<UAudioComponent>& Loop, double& NextTryTime, EKRSound Id, const FVector* Location, bool bUI)
{
	if (Loop && IsValid(Loop))
	{
		return;
	}
	// At most one try a second, in case the file is missing or can't play.
	const double Now = FPlatformTime::Seconds();
	if (Now < NextTryTime)
	{
		return;
	}
	NextTryTime = Now + 1.0;
	Loop = Spawn(Id, Location, nullptr, bUI, 1.f);
}

void UKRAudioSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	const UKRAudioData* Data = UKRAudioData::Get(this);
	if (!World || !Data)
	{
		return; // no DA_Audio set on the GameMode: the game is silent
	}
	const float RealDt = FMath::Min(static_cast<float>(FApp::GetDeltaTime()), 0.1f);
	const bool bPaused = World->IsPaused();

	if (const UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		MasterVolume = Profile->GetMasterVolume();
		MusicVolume = Profile->GetMusicVolume();
	}

	// 1) How slowed is time? The time subsystem already eases it, so world sounds follow it directly.
	//    The music eases a bit more, and opens up fully in menus.
	const UKRTimeSubsystem* Time = World->GetSubsystem<UKRTimeSubsystem>();
	if (!bPaused)
	{
		WorldBend = Time ? Time->GetFreezeAmount() : 0.f;
	}
	const float MusicTarget = bPaused ? 0.f : WorldBend;
	MusicBend = FMath::FInterpConstantTo(MusicBend, MusicTarget, RealDt, 3.f); // 1/3 s from clear to fully muffled

	// 2) Background loops: the jukebox (a UI sound, so it plays on the main menu) and the dining room.
	FindPass();
	if (!bMusicStopped)
	{
		KeepLoopRunning(MusicLoop, MusicNextTry, EKRSound::Jukebox, nullptr, true);
	}
	if (!bPaused) // (a world sound started while paused would wait silently anyway)
	{
		KeepLoopRunning(MurmurLoop, MurmurNextTry, EKRSound::DinerMurmur, bHasPass ? &PassLocation : nullptr, false);
	}

	// 3) Bend every playing sound, and forget the ones that finished.
	for (int32 i = Active.Num() - 1; i >= 0; --i)
	{
		if (!Active[i].Component.IsValid())
		{
			Active.RemoveAtSwap(i);
			continue;
		}
		ApplyBend(Active[i], Data, !bPaused);
	}

	// 4) The watch, the ramps, footsteps and the heartbeat: only while actually playing.
	const APlayerController* PC = World->GetFirstPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (bPaused || !Pawn)
	{
		return;
	}
	TickTimeSounds(Data, Pawn);
	TickPlayerSounds(Data, Pawn, RealDt);
}

void UKRAudioSubsystem::TickTimeSounds(const UKRAudioData* Data, APawn* Pawn)
{
	const UKRTimeSubsystem* Time = GetWorld()->GetSubsystem<UKRTimeSubsystem>();
	if (!Time)
	{
		return;
	}

	// Grandma's watch: one tick per second of WORLD time, so it slows down and stops with time.
	const float WorldNow = Time->GetWorldSecondsElapsed();
	if (LastWorldSeconds < 0.f)
	{
		LastWorldSeconds = WorldNow;
	}
	TickAccumulator += WorldNow - LastWorldSeconds;
	LastWorldSeconds = WorldNow;
	if (TickAccumulator >= Data->TickInterval)
	{
		TickAccumulator = FMath::Fmod(TickAccumulator, Data->TickInterval);
		Spawn(bNextIsTock ? EKRSound::WatchTock : EKRSound::WatchTick, nullptr, nullptr, false, 1.f);
		bNextIsTock = !bNextIsTock;
	}

	// Tape-stop when you stop walking, spin-up when you start: the same "moving" test the time rule uses.
	// Action bursts (chop, throw) don't count, since you're standing still for those.
	const bool bMoving = Time->IsWalkingTime();
	if (bMoving != bTimeRunning)
	{
		bTimeRunning = bMoving;
		const double Now = FPlatformTime::Seconds();
		if (Now - LastRampSoundTime >= Data->RampSoundCooldown)
		{
			LastRampSoundTime = Now;
			Spawn(bMoving ? EKRSound::TimeResume : EKRSound::TimeSlow, nullptr, nullptr, false, 1.f);
		}
	}
}

void UKRAudioSubsystem::TickPlayerSounds(const UKRAudioData* Data, APawn* Pawn, float RealDt)
{
	// Footsteps: one every StepDistance cm walked on the ground. The player moves in real time.
	const ACharacter* Character = Cast<ACharacter>(Pawn);
	const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
	const float Speed = Pawn->GetVelocity().Size2D();
	const bool bCrouched = Character && Character->bIsCrouched;
	const float Stride = bCrouched ? Data->CrouchStepDistance : Data->StepDistance;
	if (Movement && Movement->IsMovingOnGround() && Speed > GetDefault<UKRTimeSettings>()->MovingSpeedThreshold)
	{
		StepAccumulator += Speed * RealDt;
		if (StepAccumulator >= Stride)
		{
			StepAccumulator -= Stride;
			Spawn(EKRSound::Footstep, nullptr, nullptr, false, bCrouched ? Data->CrouchStepVolume : 1.f);
		}
	}
	else
	{
		StepAccumulator = Stride * 0.6f; // so the first step comes soon after you start walking
	}

	// Heartbeat while health is low, louder the lower it gets.
	if (const UKRHealthComponent* Health = Pawn->FindComponentByClass<UKRHealthComponent>())
	{
		const float Percent = Health->GetHealthPercent();
		const bool bLow = Percent > 0.f && Percent < Data->LowHealthPercent && !bMusicStopped;
		UpdateLoop(this, HeartbeatLoop, EKRSound::Heartbeat, bLow, nullptr);
		if (bLow && HeartbeatLoop)
		{
			const float Danger = 1.f - Percent / FMath::Max(Data->LowHealthPercent, 0.01f); // 0 at the threshold, 1 near death
			SetVolumeScale(HeartbeatLoop, FMath::Lerp(0.4f, 1.f, Danger));
		}
	}
}
