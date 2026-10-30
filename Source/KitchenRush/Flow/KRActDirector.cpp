// KRActDirector.cpp

#include "Flow/KRActDirector.h"
#include "Flow/KRProfileSubsystem.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRPan.h"
#include "Cooking/KRPot.h"
#include "Cooking/KRTuningData.h"
#include "Cooking/KRTypes.h"
#include "Hazards/KRGusDirector.h"
#include "Interaction/KRItem.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HAL/IConsoleManager.h"
#include "KitchenRushGameMode.h"
#include "KitchenRushPlayerController.h"

AKRActDirector::AKRActDirector()
{
	PrimaryActorTick.bCanEverTick = true;

	NoteTitle         = NSLOCTEXT("KitchenRush", "NoteTitle", "Nana's Note");
	NoteStep          = NSLOCTEXT("KitchenRush", "NoteStep", "Time waits for a cook who stands still.\nTake a step.");
	NoteTomatoSalad   = NSLOCTEXT("KitchenRush", "NoteTomatoSalad", "First order: Tomato Salad.\nGrab a tomato from its crate, chop it on a board (E, four times: even a knife costs time), put it on a plate and serve it at the pass.");
	NoteSteak         = NSLOCTEXT("KitchenRush", "NoteSteak", "Steak next. Put it in the pan on the stove.\nFood only cooks while you move. Pace if you must, and take it off while the meter is green.");
	NoteBurnt         = NSLOCTEXT("KitchenRush", "NoteBurnt", "Burnt! Bin it and grab another.\nThe crate never runs out.");
	NoteGardenSalad   = NSLOCTEXT("KitchenRush", "NoteGardenSalad", "Garden Salad: lettuce and tomato, both chopped.\nPlan the route while the world waits.");
	NoteHeadsUp       = NSLOCTEXT("KitchenRush", "NoteHeadsUp", "Heads up! Gus is tossing you an onion.\nLook at it and press E to catch it.");
	NoteThrowSteak    = NSLOCTEXT("KitchenRush", "NoteThrowSteak", "Toss it, don't walk it.\nGrab a steak, hold the left mouse button to aim, and throw it into the pan.");
	NoteBreathe       = NSLOCTEXT("KitchenRush", "NoteBreathe", "Breathe. Look.\nEverything waits while you stand still. Step out of the knives' path, then catch what you can.");
	NotePotOpens      = NSLOCTEXT("KitchenRush", "NotePotOpens", "My pot is out: Tomato Soup is on the menu.\nTwo chopped tomatoes in the pot, let it simmer, then ladle it into a bowl before it boils over.");
	NoteChainReaction = NSLOCTEXT("KitchenRush", "NoteChainReaction", "The soup's about to boil over and you can't get there.\nCan't get there? Make something get there: throw anything at the pot to knock it off the burner.");
	NoteShelf         = NSLOCTEXT("KitchenRush", "NoteShelf", "The shelf is going!\nGrab a plate and catch the tomatoes right onto it. Mind the knives.");
	GusDoorsOpen      = NSLOCTEXT("KitchenRush", "GusDoorsOpen", "Gus: Doors are open, chef! Here they come!");
	GusSpecial        = NSLOCTEXT("KitchenRush", "GusSpecial", "Gus: Nana's Special! Steak and Onions with a bowl of soup!");
	WinReason         = NSLOCTEXT("KitchenRush", "WinReason", "10:00 PM. You survived the Friday rush!");
}

AKRActDirector* AKRActDirector::Find(const UObject* WorldContext)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (World)
	{
		for (TActorIterator<AKRActDirector> It(World); It; ++It)
		{
			return *It;
		}
	}
	return nullptr;
}

AKitchenRushGameMode* AKRActDirector::GetGameMode() const
{
	return GetWorld()->GetAuthGameMode<AKitchenRushGameMode>();
}

AKitchenRushPlayerController* AKRActDirector::GetPC() const
{
	return Cast<AKitchenRushPlayerController>(GetWorld()->GetFirstPlayerController());
}

void AKRActDirector::BeginPlay()
{
	Super::BeginPlay();

	// Take control of Gus and the orders: nothing happens until the night starts.
	Gus = AKRGusDirector::Find(GetWorld());
	if (AKRGusDirector* G = Gus.Get())
	{
		G->SetAutoEvents(false);
		G->OnEventLaunched.AddUObject(this, &AKRActDirector::HandleGusLaunched);
	}
	if (AKitchenRushGameMode* GameMode = GetGameMode())
	{
		GameMode->SetOrdersEnabled(false);
		GameMode->OnGameEvent.AddUObject(this, &AKRActDirector::HandleEvent);
	}
	for (AActor* Actor : UnlockInAct3)
	{
		SetLocked(Actor, true);
	}

	// Wait one frame so the player controller and HUD exist before we show a menu or a note.
	GetWorldTimerManager().SetTimerForNextTick(this, &AKRActDirector::BeginNight);
}

void AKRActDirector::BeginNight()
{
	// OpenLevel(..., "Act=2") from Continue / Restart act / Play again lands here with an option.
	const AKitchenRushGameMode* GameMode = GetGameMode();
	const FString Options = GameMode ? GameMode->OptionsString : FString();
	if (UGameplayStatics::HasOption(Options, TEXT("Act")))
	{
		StartNight(UGameplayStatics::GetIntOption(Options, TEXT("Act"), 1));
		return;
	}
#if WITH_EDITOR
	if (GetWorld()->IsPlayInEditor() && EditorStartAct > 0)
	{
		StartNight(EditorStartAct);
		return;
	}
#endif
	Beat = EKRBeat::MainMenu;
	if (AKitchenRushPlayerController* PC = GetPC())
	{
		PC->ShowMainMenu();
	}
}

void AKRActDirector::StartNight(int32 Act)
{
	Act = FMath::Clamp(Act, 1, 3);
	if (AKitchenRushGameMode* GameMode = GetGameMode())
	{
		GameMode->SetFullRun(Act == 1);
	}
	StartAct(Act);
}

float AKRActDirector::HoursToShiftSeconds(float Hours) const
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	return (Hours - T->ShiftStartHour) * T->WorldSecondsPerGameHour;
}

float AKRActDirector::GetClockHours() const
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	return T->ShiftStartHour + ShiftSeconds / FMath::Max(T->WorldSecondsPerGameHour, 1.f);
}

void AKRActDirector::SetClockHours(float Hours)
{
	if (bClockRunning)
	{
		ShiftSeconds = FMath::Max(0.f, HoursToShiftSeconds(Hours));
	}
}

void AKRActDirector::ShowActTitle(const FText& Title)
{
	ActTitle = Title;
	ActTitleTime = GetWorld()->GetRealTimeSeconds();
}

void AKRActDirector::Note(const FText& Body)
{
	if (bTutorialSkipped || Body.IsEmpty())
	{
		return;
	}
	if (AKitchenRushPlayerController* PC = GetPC())
	{
		PC->ShowNote(NoteTitle, Body);
	}
}

void AKRActDirector::SetLocked(AActor* Actor, bool bLocked)
{
	if (!Actor)
	{
		return;
	}
	Actor->SetActorHiddenInGame(bLocked);
	Actor->SetActorEnableCollision(!bLocked);
	// A hidden item mustn't fall through the floor (collision is off), so its physics is paused too.
	if (AKRItem* Item = Cast<AKRItem>(Actor))
	{
		if (UStaticMeshComponent* Mesh = Item->GetMesh())
		{
			Mesh->SetSimulatePhysics(!bLocked);
		}
	}
}

// ============================================================================
// Acts
// ============================================================================

void AKRActDirector::StartAct(int32 Act)
{
	AKitchenRushGameMode* GameMode = GetGameMode();
	AKRGusDirector* G = Gus.Get();
	if (!GameMode)
	{
		return;
	}
	const bool bWasRushing = (Beat == EKRBeat::Act2_Rush); // Act 2 flowing into Act 3
	CurrentAct = Act;

	if (UKRProfileSubsystem* Profile = UKRProfileSubsystem::Get(this))
	{
		Profile->RecordAct(Act);
	}
	GameMode->BroadcastEvent(EKRGameEvent::ActStarted, NAME_None, float(Act));

	if (Act == 1)
	{
		Beat = EKRBeat::Act1_Step;
		GameMode->SetTutorialMode(true);
		GameMode->SetOrdersEnabled(false);
		GameMode->ClearOrders();
		bClockRunning = false;
		ShiftSeconds = 0.f;
		ShowActTitle(NSLOCTEXT("KitchenRush", "Act1Title", "ACT 1 · NANA'S NOTES"));

		// The opening image: a tomato hanging in mid-air, waiting for you to move.
		if (IntroTomatoSpot && IntroTomatoClass)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			GetWorld()->SpawnActor<AKRFood>(IntroTomatoClass, IntroTomatoSpot->GetActorLocation(), FRotator::ZeroRotator, Params);
		}
		if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			StepStart = Player->GetActorLocation();
		}
		Note(NoteStep);
		return;
	}

	if (Act == 2)
	{
		GameMode->SetTutorialMode(false);
		GameMode->ClearOrders();
		ShowActTitle(NSLOCTEXT("KitchenRush", "Act2Title", "ACT 2 · THE DINNER RUSH"));
		GameMode->ShowMessage(GusDoorsOpen, 4.f);
		if (bTutorialSkipped)
		{
			BeginRush2();
			return;
		}
		// "Heads up!": catch Gus's onion, then throw a steak into the pan. The clock waits for these.
		Beat = EKRBeat::Act2_CatchOnion;
		Note(NoteHeadsUp);
		TossOnion();
		return;
	}

	// ---- Act 3 ----
	if (!bWasRushing)
	{
		// Started straight at Act 3 (Continue / Restart act): set up what Act 2 would have.
		GameMode->SetTutorialMode(false);
		GameMode->ClearOrders();
		GameMode->SetMenu({ TomatoSaladRow, GardenSaladRow, SteakOnionsRow });
		GameMode->SetOrdersEnabled(true);
		ShiftSeconds = HoursToShiftSeconds(Act3Hour);
		bClockRunning = true;
		bTrip1Done = true;
		bTrip2Done = true;
	}
	Beat = EKRBeat::Act3_Rush;
	ShowActTitle(NSLOCTEXT("KitchenRush", "Act3Title", "ACT 3 · CLOSING TIME"));
	for (AActor* Actor : UnlockInAct3)
	{
		SetLocked(Actor, false); // the pot opens
	}
	GameMode->AddToMenu(TomatoSoupRow);
	GameMode->SetMaxTickets(3);
	if (G)
	{
		G->SetAutoToss(true);
		G->SetAutoTrips(true);
	}
	Note(NotePotOpens);
}

void AKRActDirector::BeginRush2()
{
	AKitchenRushGameMode* GameMode = GetGameMode();
	Beat = EKRBeat::Act2_Rush;
	bClockRunning = true;
	ShiftSeconds = 0.f; // 5:00 PM
	if (GameMode)
	{
		GameMode->SetMenu({ TomatoSaladRow, GardenSaladRow, SteakOnionsRow });
		GameMode->SetMaxTickets(2);
		GameMode->SetOrdersEnabled(true);
	}
	if (AKRGusDirector* G = Gus.Get())
	{
		G->SetAutoToss(true);
		G->SetAutoTrips(false); // timed trips start after the 6:15 teaching trip
	}
}

void AKRActDirector::TossOnion()
{
	AKRGusDirector* G = Gus.Get();
	TSubclassOf<AKRFood> OnionClass = G ? G->FindFoodClass(EKRFoodType::Onion) : nullptr;
	if (!OnionClass)
	{
		// No Gus or no onion set up: skip the catch lesson rather than get stuck.
		Beat = EKRBeat::Act2_ThrowSteak;
		Note(NoteThrowSteak);
		return;
	}
	TutorialOnion = G->LaunchItem(OnionClass, EKRGusAim::InFrontOfPlayer);
	OnionGoneSeconds = 0.f;
}

void AKRActDirector::SkipTutorial()
{
	if (bTutorialSkipped)
	{
		return;
	}
	bTutorialSkipped = true;
	FreezeCountdown = -1.f;
	AwaitedEvent = NAME_None;
	if (AKitchenRushGameMode* GameMode = GetGameMode())
	{
		GameMode->BroadcastEvent(EKRGameEvent::TutorialSkipped);
	}
	if (CurrentAct == 1)
	{
		StartAct(2); // straight to "Doors are open"
	}
	else if (Beat == EKRBeat::Act2_CatchOnion || Beat == EKRBeat::Act2_ThrowSteak)
	{
		BeginRush2();
	}
}

void AKRActDirector::PlayScriptedTrip(const FText& InFreezeNote)
{
	AKRGusDirector* G = Gus.Get();
	if (!G || !G->PlayEvent(TripEvent))
	{
		return;
	}
	G->ResetTripTimer(); // no timed trip right on top of this one
	if (!InFreezeNote.IsEmpty() && !bTutorialSkipped)
	{
		AwaitedEvent = TripEvent;
		FreezeNote = InFreezeNote;
	}
}

void AKRActDirector::HandleGusLaunched(FName EventName)
{
	if (EventName == ShelfEvent)
	{
		DropShelfBoards();
	}
	if (!AwaitedEvent.IsNone() && EventName == AwaitedEvent)
	{
		AwaitedEvent = NAME_None;
		FreezeCountdown = AutoFreezeDelay; // Tick shows the note when it runs out
	}
}

void AKRActDirector::DropShelfBoards()
{
	if (ShelfBoardTag.IsNone())
	{
		return;
	}
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (!It->ActorHasTag(ShelfBoardTag))
		{
			continue;
		}
		UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(It->GetRootComponent());
		if (Body && Body->Mobility == EComponentMobility::Movable)
		{
			Body->SetSimulatePhysics(true);
			Body->AddImpulse(FVector(0.f, 0.f, -50.f), NAME_None, true);
		}
	}
}

// ============================================================================
// Every frame (world time: everything here waits while you stand still)
// ============================================================================

void AKRActDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const AKitchenRushGameMode* GameMode = GetGameMode();
	if (!GameMode || GameMode->IsGameOver() || Beat == EKRBeat::Done || Beat == EKRBeat::None || Beat == EKRBeat::MainMenu)
	{
		return;
	}

	if (bClockRunning)
	{
		ShiftSeconds += DeltaSeconds;
	}

	// Auto-freeze: the note pauses the game at the peak of the burst.
	if (FreezeCountdown >= 0.f)
	{
		FreezeCountdown -= DeltaSeconds;
		if (FreezeCountdown < 0.f)
		{
			Note(FreezeNote);
		}
	}

	switch (CurrentAct)
	{
	case 1: UpdateAct1(); break;
	case 2: UpdateAct2(DeltaSeconds); break;
	case 3: UpdateAct3(); break;
	default: break;
	}

	// Keep Gus's timed trips clear of the scripted ones.
	if (AKRGusDirector* G = Gus.Get())
	{
		const float Until = SecondsToNextScriptedBeat();
		if (Until > 0.f)
		{
			G->KeepAutoTripAwayFrom(Until);
		}
	}
}

void AKRActDirector::UpdateAct1()
{
	if (Beat == EKRBeat::Act1_Step)
	{
		const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
		if (Player && FVector::Dist2D(Player->GetActorLocation(), StepStart) > 80.f)
		{
			Beat = EKRBeat::Act1_TomatoSalad;
			if (AKitchenRushGameMode* GameMode = GetGameMode())
			{
				GameMode->AddOrder(TomatoSaladRow, -1.f); // no patience bar yet
			}
			Note(NoteTomatoSalad);
		}
	}
}

void AKRActDirector::UpdateAct2(float DeltaSeconds)
{
	if (Beat == EKRBeat::Act2_CatchOnion)
	{
		// Missed it? Once it's lying somewhere (or got binned), Gus throws another.
		const AKRItem* Onion = TutorialOnion.Get();
		const bool bLanded = !Onion || (!Onion->IsInFlight() && !Onion->IsHeld() && !Onion->IsStored());
		OnionGoneSeconds = bLanded ? OnionGoneSeconds + DeltaSeconds : 0.f;
		if (OnionGoneSeconds > 1.5f)
		{
			if (AKitchenRushGameMode* GameMode = GetGameMode())
			{
				GameMode->ShowMessage(NSLOCTEXT("KitchenRush", "GusAnother", "Gus: Whoops! Here's another one."));
			}
			TossOnion();
		}
		return;
	}
	if (Beat != EKRBeat::Act2_Rush)
	{
		return;
	}

	if (!bTrip1Done && ShiftSeconds >= HoursToShiftSeconds(FirstTripHour))
	{
		bTrip1Done = true;
		PlayScriptedTrip(NoteBreathe); // the teaching trip: auto-freeze with "Breathe. Look."
		if (AKRGusDirector* G = Gus.Get())
		{
			G->SetAutoTrips(true); // timed trips from now on
		}
	}
	if (!bTrip2Done && ShiftSeconds >= HoursToShiftSeconds(SecondTripHour))
	{
		bTrip2Done = true;
		PlayScriptedTrip(FText::GetEmpty()); // no help this time
	}
	if (ShiftSeconds >= HoursToShiftSeconds(Act3Hour))
	{
		StartAct(3);
	}
}

void AKRActDirector::UpdateAct3()
{
	AKitchenRushGameMode* GameMode = GetGameMode();
	AKRPot* Pot = nullptr;
	for (TActorIterator<AKRPot> It(GetWorld()); It && !Pot; ++It) { Pot = *It; }
	// [M4.1] The pacing puzzle has no note any more: players work out the micro-bursts themselves.

	// Chain reaction: Gus trips while the soup is about to boil over.
	if (!bChainDone && ShiftSeconds >= HoursToShiftSeconds(ChainReactionHour))
	{
		const UKRTuningData* T = UKRTuningData::Get(this);
		if (Pot && Pot->IsSimmering())
		{
			bChainDone = true;
			// Boil-over lands just after Gus's warning and the freeze, so there's no time to walk there.
			Pot->FastForwardToBoilOver(T->TripWarningSeconds + ChainReactionBoilSeconds);
			PlayScriptedTrip(NoteChainReaction);
		}
		else if (ShiftSeconds >= HoursToShiftSeconds(ChainReactionLatestHour))
		{
			bChainDone = true; // no soup on: Gus still trips, without the lesson
			PlayScriptedTrip(FText::GetEmpty());
		}
	}

	// Shelf collapse.
	if (!bShelfDone && ShiftSeconds >= HoursToShiftSeconds(ShelfCollapseHour))
	{
		bShelfDone = true;
		AKRGusDirector* G = Gus.Get();
		if (G && G->HasEvent(ShelfEvent) && G->PlayEvent(ShelfEvent))
		{
			G->ResetTripTimer();
			Note(NoteShelf);
		}
	}

	// Nana's Special: the combo order.
	if (!bSpecialDone && ShiftSeconds >= HoursToShiftSeconds(SpecialHour) && GameMode)
	{
		bSpecialDone = true;
		GameMode->AddOrder(SteakOnionsRow, 0.f);
		GameMode->AddOrder(TomatoSoupRow, 0.f);
		GameMode->ShowMessage(GusSpecial, 4.f);
	}

	// 10:00 PM: you made it.
	if (ShiftSeconds >= HoursToShiftSeconds(EndHour) && GameMode)
	{
		Beat = EKRBeat::Done;
		GameMode->EndShift(true, WinReason, EKRRunOutcome::Won);
	}
}

float AKRActDirector::SecondsToNextScriptedBeat() const
{
	float Next = -1.f;
	auto Consider = [this, &Next](bool bDone, float Hour)
	{
		const float Until = HoursToShiftSeconds(Hour) - ShiftSeconds;
		if (!bDone && Until > 0.f && (Next < 0.f || Until < Next))
		{
			Next = Until;
		}
	};
	Consider(bTrip2Done, SecondTripHour);
	Consider(bChainDone, ChainReactionHour);
	Consider(bShelfDone, ShelfCollapseHour);
	return Next;
}

// ============================================================================
// Events from the GameMode
// ============================================================================

void AKRActDirector::HandleEvent(const FKRGameEvent& Event)
{
	AKitchenRushGameMode* GameMode = GetGameMode();
	switch (Event.Type)
	{
	case EKRGameEvent::DishServed:
		if (Beat == EKRBeat::Act1_TomatoSalad && Event.Name == TomatoSaladRow)
		{
			Beat = EKRBeat::Act1_Steak;
			if (GameMode)
			{
				GameMode->AddOrder(PlainSteakRow, TutorialSteakPatience); // the first patience bar
			}
			Note(NoteSteak);
		}
		else if (Beat == EKRBeat::Act1_Steak && Event.Name == PlainSteakRow)
		{
			Beat = EKRBeat::Act1_GardenSalad;
			if (GameMode)
			{
				GameMode->AddOrder(GardenSaladRow, 0.f); // normal patience
			}
			Note(NoteGardenSalad);
		}
		else if (Beat == EKRBeat::Act1_GardenSalad && Event.Name == GardenSaladRow)
		{
			StartAct(2);
		}
		break;

	case EKRGameEvent::SteakBurnt:
		if (CurrentAct == 1 && !bShownBurnt)
		{
			bShownBurnt = true;
			Note(NoteBurnt);
		}
		break;

	case EKRGameEvent::Catch:
		if (Beat == EKRBeat::Act2_CatchOnion)
		{
			Beat = EKRBeat::Act2_ThrowSteak;
			Note(NoteThrowSteak);
		}
		break;

	case EKRGameEvent::TrickShot:
		if (Beat == EKRBeat::Act2_ThrowSteak && (Event.Flags & KRTrick::Pan) != 0)
		{
			BeginRush2();
			// [M4.1] The lesson just gave you an onion and a steak in the pan: the first ticket uses them.
			if (GameMode)
			{
				GameMode->AddOrder(SteakOnionsRow, 0.f);
			}
		}
		break;

	default:
		break;
	}
}

// ============================================================================
// Testing console commands (press ` while playing)
// ============================================================================

static void KRCmdAct(const TArray<FString>& Args, UWorld* World)
{
	if (AKitchenRushPlayerController* PC = World ? Cast<AKitchenRushPlayerController>(World->GetFirstPlayerController()) : nullptr)
	{
		PC->RestartAtAct(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1);
	}
}

static void KRCmdClock(const TArray<FString>& Args, UWorld* World)
{
	if (AKRActDirector* Director = AKRActDirector::Find(World))
	{
		if (Args.Num() > 0)
		{
			Director->SetClockHours(FCString::Atof(*Args[0]));
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs GKRAct(TEXT("kr.Act"), TEXT("Restart the night at an act: kr.Act 3"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&KRCmdAct));
static FAutoConsoleCommandWithWorldAndArgs GKRClock(TEXT("kr.Clock"), TEXT("Jump the night's clock (hours, 24h): kr.Clock 20.4 = 8:24 PM"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&KRCmdClock));
