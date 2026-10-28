// KRTimeSubsystem.cpp

#include "Time/KRTimeSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"

void UKRTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Start each level with the modes chosen in Project Settings
	const UKRTimeSettings* Settings = GetDefault<UKRTimeSettings>();
	StillMode = Settings->StillMode;
	MoveMode = Settings->MoveMode;
	CurrentScale = 1.f;
}

void UKRTimeSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

TStatId UKRTimeSubsystem::GetStatId() const
{
	// Lets Unreal's profiler show how long this tick takes
	RETURN_QUICK_DECLARE_CYCLE_STAT(UKRTimeSubsystem, STATGROUP_Tickables);
}

bool UKRTimeSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UKRTimeSubsystem::TriggerActionBurst()
{
	BurstSecondsLeft = FMath::Max(BurstSecondsLeft, GetDefault<UKRTimeSettings>()->ActionBurstSeconds);
}

float UKRTimeSubsystem::GetFreezeAmount() const
{
	const UKRTimeSettings* S = GetDefault<UKRTimeSettings>();
	const float StillScale = (StillMode == EKRStillMode::Freeze) ? S->FreezeScale : S->SlowScale;
	const float Range = S->WalkScale - StillScale;
	if (Range <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	// 1 when CurrentScale is at the "still" speed, 0 at walk speed or faster
	return FMath::Clamp((S->WalkScale - CurrentScale) / Range, 0.f, 1.f);
}

// The speed the world runs at while walking (Clamped or Momentum).
float UKRTimeSubsystem::ComputeMoveScale() const
{
	const UKRTimeSettings* S = GetDefault<UKRTimeSettings>();
	if (MoveMode == EKRMoveMode::Momentum)
	{
		const float Ramp = FMath::Clamp(MovingSeconds / S->MomentumRampSeconds, 0.f, 1.f);
		return FMath::Lerp(S->WalkScale, S->MomentumMaxScale, Ramp);
	}
	return S->WalkScale;
}

void UKRTimeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		return; // no player yet
	}

	const UKRTimeSettings* S = GetDefault<UKRTimeSettings>();

	// FApp::GetDeltaTime() is the real frame time
	const float RealDt = FMath::Min(static_cast<float>(FApp::GetDeltaTime()), 0.1f);

	// 1) Is the player moving? Velocity is in cm/s. Size2D ignores up/down
	const bool bMoving = Pawn->GetVelocity().Size2D() > S->MovingSpeedThreshold;
	MovingSeconds = bMoving ? MovingSeconds + RealDt : 0.f;
	BurstSecondsLeft = FMath::Max(0.f, BurstSecondsLeft - RealDt);

	// 2) Ramp "Flow" between 0 (still) and 1 (walking) at a FIXED speed
	if (bMoving || BurstSecondsLeft > 0.f)
	{
		const float RampSeconds = bMoving ? S->SpeedUpSeconds : S->ActionRampSeconds;
		Flow = FMath::Min(1.f, Flow + RealDt / RampSeconds);
	}
	else
	{
		Flow = FMath::Max(0.f, Flow - RealDt / S->SlowDownSeconds);
	}

	// SmoothStep makes the ramp start gently and end gently (an S-curve), instead of a straight line.
	const float Eased = FMath::SmoothStep(0.f, 1.f, Flow);
	const float StillScale = (StillMode == EKRStillMode::Freeze) ? S->FreezeScale : S->SlowScale;
	CurrentScale = FMath::Lerp(StillScale, ComputeMoveScale(), Eased);

	// 3) Slow down the whole world
	UGameplayStatics::SetGlobalTimeDilation(World, CurrentScale);
	const float Applied = World->GetWorldSettings()->GetEffectiveTimeDilation();

	// 4) then speed the player back up, so: world speed x player multiplier = 1. Example: 0.0001 x 10000 = 1
	const float Compensation = 1.f / FMath::Max(Applied, 0.0001f);
	Pawn->CustomTimeDilation = Compensation;
	PC->CustomTimeDilation = Compensation;

	WorldSeconds += RealDt * Applied;

	// 5) Debug readout (turn off in Project Settings > Kitchen Rush Time > Debug)
	if (S->bShowDebugText && GEngine)
	{
		const TCHAR* StillName = (StillMode == EKRStillMode::Freeze) ? TEXT("Freeze") : TEXT("Slow");
		const TCHAR* MoveName = (MoveMode == EKRMoveMode::Clamped) ? TEXT("Clamped") : TEXT("Momentum");
		GEngine->AddOnScreenDebugMessage(4242, 0.f, FColor::Yellow,
			FString::Printf(TEXT("Time x%.4f   (%s / %s)   world time %.1f s"), Applied, StillName, MoveName, WorldSeconds));
	}
}

// ---- Console commands for A/B testing ----
// While playing, press the ` key (left of 1) to open the console and type:
//     kr.Still freeze     kr.Still slow
//     kr.Move clamped     kr.Move momentum

static FAutoConsoleCommandWithWorldAndArgs GKRStillCommand(
	TEXT("kr.Still"),
	TEXT("What time does while standing still: kr.Still freeze | kr.Still slow"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UKRTimeSubsystem* Time = World ? World->GetSubsystem<UKRTimeSubsystem>() : nullptr;
		if (Time && Args.Num() > 0)
		{
			Time->SetStillMode(Args[0].Equals(TEXT("slow"), ESearchCase::IgnoreCase) ? EKRStillMode::Slow : EKRStillMode::Freeze);
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs GKRMoveCommand(
	TEXT("kr.Move"),
	TEXT("What time does while walking: kr.Move clamped | kr.Move momentum"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UKRTimeSubsystem* Time = World ? World->GetSubsystem<UKRTimeSubsystem>() : nullptr;
		if (Time && Args.Num() > 0)
		{
			Time->SetMoveMode(Args[0].Equals(TEXT("momentum"), ESearchCase::IgnoreCase) ? EKRMoveMode::Momentum : EKRMoveMode::Clamped);
		}
	}));
