// KRAudioData.cpp

#include "Audio/KRAudioData.h"
#include "Engine/World.h"
#include "Sound/SoundBase.h"
#include "KitchenRushGameMode.h"

USoundBase* FKRSound::Pick() const
{
	// Gather every file that's actually set, then pick one.
	TArray<USoundBase*, TInlineAllocator<4>> Choices;
	if (Sound)
	{
		Choices.Add(Sound);
	}
	for (USoundBase* Extra : Variations)
	{
		if (Extra)
		{
			Choices.Add(Extra);
		}
	}
	if (Choices.Num() == 0)
	{
		return nullptr;
	}
	return Choices[FMath::RandRange(0, Choices.Num() - 1)];
}

const UKRAudioData* UKRAudioData::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	const AKitchenRushGameMode* GameMode = World ? World->GetAuthGameMode<AKitchenRushGameMode>() : nullptr;
	return GameMode ? GameMode->GetAudio() : nullptr;
}

const FKRSound* UKRAudioData::Find(EKRSound Id) const
{
	switch (Id)
	{
	case EKRSound::WatchTick:   return &WatchTick;
	case EKRSound::WatchTock:   return &WatchTock;
	case EKRSound::TimeSlow:    return &TimeSlow;
	case EKRSound::TimeResume:  return &TimeResume;
	case EKRSound::DinerMurmur: return &DinerMurmur;
	case EKRSound::Jukebox:     return &Jukebox;
	case EKRSound::Win:         return &Win;
	case EKRSound::GameOver:    return &GameOver;
	case EKRSound::Footstep:    return &Footstep;
	case EKRSound::Throw:       return &Throw;
	case EKRSound::Hurt:        return &Hurt;
	case EKRSound::Heartbeat:   return &Heartbeat;
	case EKRSound::Chop:        return &Chop;
	case EKRSound::Sizzle:      return &Sizzle;
	case EKRSound::CookDone:    return &CookDone;
	case EKRSound::FireFlare:   return &FireFlare;
	case EKRSound::FireLoop:    return &FireLoop;
	case EKRSound::PotBubble:   return &PotBubble;
	case EKRSound::Plop:        return &Plop;
	case EKRSound::BoilOver:    return &BoilOver;
	case EKRSound::DishClink:   return &DishClink;
	case EKRSound::TrashDump:   return &TrashDump;
	case EKRSound::Thud:        return &Thud;
	case EKRSound::PanClang:    return &PanClang;
	case EKRSound::PotClang:    return &PotClang;
	case EKRSound::GusHeadsUp:  return &GusHeadsUp;
	case EKRSound::GusWatchOut: return &GusWatchOut;
	case EKRSound::KnifeWhoosh: return &KnifeWhoosh;
	case EKRSound::Shatter:     return &Shatter;
	case EKRSound::ShardCrunch: return &ShardCrunch;
	case EKRSound::Splat:       return &Splat;
	case EKRSound::ShelfCreak:  return &ShelfCreak;
	case EKRSound::ShelfCrash:  return &ShelfCrash;
	case EKRSound::NewOrder:    return &NewOrder;
	case EKRSound::Served:      return &Served;
	case EKRSound::NoteRustle:  return &NoteRustle;
	case EKRSound::UIClick:     return &UIClick;
	default:                    return nullptr;
	}
}
