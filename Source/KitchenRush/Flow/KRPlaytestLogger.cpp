// KRPlaytestLogger.cpp

#include "Flow/KRPlaytestLogger.h"
#include "Time/KRTimeSubsystem.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "KitchenRushGameMode.h"

bool UKRPlaytestLogger::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UKRPlaytestLogger::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	// The GameMode exists by now: listen to its events.
	if (AKitchenRushGameMode* GameMode = InWorld.GetAuthGameMode<AKitchenRushGameMode>())
	{
		GameMode->OnGameEvent.AddUObject(this, &UKRPlaytestLogger::HandleEvent);
	}
}

void UKRPlaytestLogger::HandleEvent(const FKRGameEvent& Event)
{
	const UWorld* World = GetWorld();
	switch (Event.Type)
	{
	case EKRGameEvent::ActStarted:
	{
		const int32 Act = FMath::Clamp(FMath::RoundToInt(Event.Value), 1, 3);
		if (!bRunStarted)
		{
			bRunStarted = true;
			StartAct = Act;
			RunRealStart = FPlatformTime::Seconds();
		}
		else
		{
			CloseAct();
		}
		CurrentAct = Act;
		ActRealStart = FPlatformTime::Seconds();
		ActWorldStart = World ? World->GetTimeSeconds() : 0.0; // GetTimeSeconds = world time (slowed down, stops when paused)
		break;
	}
	case EKRGameEvent::Strike:          ++Strikes; break;
	case EKRGameEvent::SteakBurnt:      ++BurntSteaks; break;
	case EKRGameEvent::Catch:           ++Catches; break;
	case EKRGameEvent::KnifeHit:        ++KnifeHits; break;
	case EKRGameEvent::TrickShot:       ++TrickShots; break;
	case EKRGameEvent::DishServed:      ++Served; break;
	case EKRGameEvent::TutorialSkipped: bTutorialSkipped = true; break;
	case EKRGameEvent::RunEnded:
		Outcome = static_cast<EKRRunOutcome>(Event.Flags);
		FinalTips = FMath::RoundToInt(Event.Value);
		CloseAct();
		WriteRow();
		break;
	default:
		break;
	}
}

void UKRPlaytestLogger::CloseAct()
{
	if (CurrentAct < 1 || CurrentAct > 3)
	{
		return;
	}
	const UWorld* World = GetWorld();
	RealSeconds[CurrentAct] += FPlatformTime::Seconds() - ActRealStart;
	WorldSeconds[CurrentAct] += (World ? World->GetTimeSeconds() : 0.0) - ActWorldStart;
	CurrentAct = 0;
}

void UKRPlaytestLogger::Deinitialize()
{
	// Level closing mid-run (quit, restart, main menu): log it as "Quit".
	if (bRunStarted && !bWritten)
	{
		CloseAct();
		if (const AKitchenRushGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AKitchenRushGameMode>() : nullptr)
		{
			FinalTips = GameMode->GetTips();
		}
		WriteRow();
	}
	Super::Deinitialize();
}

void UKRPlaytestLogger::WriteRow()
{
	if (bWritten)
	{
		return;
	}
	bWritten = true;

	const FString Path = FPaths::ProjectSavedDir() / TEXT("Playtests") / TEXT("KitchenRush_Playtests.csv");
	const bool bNewFile = !FPaths::FileExists(Path);

	FString StillMode = TEXT("?");
	FString MoveMode = TEXT("?");
	if (const UKRTimeSubsystem* Time = GetWorld() ? GetWorld()->GetSubsystem<UKRTimeSubsystem>() : nullptr)
	{
		StillMode = (Time->GetStillMode() == EKRStillMode::Freeze) ? TEXT("Freeze") : TEXT("Slow");
		MoveMode = (Time->GetMoveMode() == EKRMoveMode::Clamped) ? TEXT("Clamped") : TEXT("Momentum");
	}
	const TCHAR* OutcomeNames[] = { TEXT("Won"), TEXT("Health"), TEXT("Strikes"), TEXT("Quit") };

	FString Text;
	if (bNewFile)
	{
		Text += TEXT("Date,Outcome,StartAct,TutorialSkipped,StillMode,MoveMode,")
			TEXT("Act1Real,Act1World,Act2Real,Act2World,Act3Real,Act3World,TotalReal,")
			TEXT("Deaths,Strikes,BurntSteaks,Catches,KnifeHits,TrickShots,Served,Tips\n");
	}
	Text += FString::Printf(TEXT("%s,%s,%d,%s,%s,%s,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%.0f,%d,%d,%d,%d,%d,%d,%d,%d\n"),
		*FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M")),
		OutcomeNames[static_cast<int32>(Outcome)],
		StartAct,
		bTutorialSkipped ? TEXT("yes") : TEXT("no"),
		*StillMode, *MoveMode,
		RealSeconds[1], WorldSeconds[1], RealSeconds[2], WorldSeconds[2], RealSeconds[3], WorldSeconds[3],
		FPlatformTime::Seconds() - RunRealStart,
		Outcome == EKRRunOutcome::Health ? 1 : 0,
		Strikes, BurntSteaks, Catches, KnifeHits, TrickShots, Served, FinalTips);

	// Append (make the folder first; the file is created if it isn't there).
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
	FFileHelper::SaveStringToFile(Text, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM, &IFileManager::Get(), FILEWRITE_Append);
}
