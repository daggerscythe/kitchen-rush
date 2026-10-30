// KRGusDirector.cpp

#include "Hazards/KRGusDirector.h"
#include "Hazards/KRHazardProjectile.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRTuningData.h"
#include "Player/KRHealthComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "KitchenRushGameMode.h"

AKRGusDirector::AKRGusDirector()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetupAttachment(Root);
	Arrow->ArrowSize = 2.f;
	Arrow->bIsScreenSizeScaled = true;

	// The default "Trip": 2 knives, 1 hot sauce, 2 steaks. The plan's Act 2.3 burst.
	FKRGusEvent Trip;
	Trip.Name = TEXT("Trip");
	Trip.Shout = NSLOCTEXT("KitchenRush", "GusTrip", "GUS IS TRIPPING! LOOK OUT!");
	auto AddRow = [&Trip](EKRGusAim Aim, float Delay)
	{
		FKRGusLaunch Launch;
		Launch.Aim = Aim;
		Launch.Delay = Delay;
		Trip.Launches.Add(Launch);
	};
	AddRow(EKRGusAim::AtPlayer, 0.0f);         // row 0: knife
	AddRow(EKRGusAim::NearPlayer, 0.25f);      // row 1: knife
	AddRow(EKRGusAim::NearPlayer, 0.15f);      // row 2: hot sauce
	AddRow(EKRGusAim::InFrontOfPlayer, 0.1f);  // row 3: steak
	AddRow(EKRGusAim::InFrontOfPlayer, 0.4f);  // row 4: steak
	Events.Add(Trip);
}

void AKRGusDirector::BeginPlay()
{
	Super::BeginPlay();
	const UKRTuningData* T = UKRTuningData::Get(this);
	SecondsToToss = T->TossInterval;
	SecondsToTrip = T->FirstTripDelay > 0.f ? T->FirstTripDelay : T->TripInterval; // [M3.5]
	WarningLocation = GetActorLocation();
	if (!bAutoEvents)
	{
		SetAutoEvents(false); // [M4] the old single switch still works
	}
}

// [M4]
void AKRGusDirector::ResetTripTimer()
{
	SecondsToTrip = UKRTuningData::Get(this)->TripInterval;
}

// [M4]
void AKRGusDirector::KeepAutoTripAwayFrom(float SecondsUntil)
{
	if (bAutoTrips && FMath::Abs(SecondsToTrip - SecondsUntil) < 12.f)
	{
		SecondsToTrip = SecondsUntil + 12.f;
	}
}

// [M4]
TSubclassOf<AKRFood> AKRGusDirector::FindFoodClass(EKRFoodType Type) const
{
	for (const TSubclassOf<AKRFood>& FoodClass : FoodClasses)
	{
		if (FoodClass && FoodClass->GetDefaultObject<AKRFood>()->FoodType == Type)
		{
			return FoodClass;
		}
	}
	return nullptr;
}

// [M4]
bool AKRGusDirector::HasEvent(FName EventName) const
{
	return Events.ContainsByPredicate([EventName](const FKRGusEvent& E) { return E.Name == EventName; });
}

// [M4] An actor with this tag, or Gus himself.
FVector AKRGusDirector::FindOrigin(FName Tag) const
{
	if (!Tag.IsNone())
	{
		for (TActorIterator<AActor> It(GetWorld()); It; ++It)
		{
			if (It->ActorHasTag(Tag))
			{
				return It->GetActorLocation();
			}
		}
	}
	return GetActorLocation();
}

// [M3.5]
void AKRGusDirector::StartWarning(const FText& Text, float Seconds)
{
	WarningText = Text;
	PendingSecondsLeft = FMath::Max(Seconds, 0.f);
	WarningHoldLeft = 0.f;
}

// [M3.5]
void AKRGusDirector::RunPending()
{
	const EPending What = Pending;
	Pending = EPending::None;
	WarningHoldLeft = (What == EPending::Event) ? 0.75f : 0.f; // [M3.6] world seconds; was 1.5

	if (What == EPending::Toss)
	{
		LaunchItem(PendingFood, EKRGusAim::InFrontOfPlayer);
	}
	else if (What == EPending::Event)
	{
		const FName Name = PendingEvent;
		if (const FKRGusEvent* Event = Events.FindByPredicate([Name](const FKRGusEvent& E) { return E.Name == Name; }))
		{
			LaunchEventRows(*Event);
			OnEventLaunched.Broadcast(Name); // [M4]
		}
	}
}

AKRGusDirector* AKRGusDirector::Find(UWorld* World)
{
	if (World)
	{
		for (TActorIterator<AKRGusDirector> It(World); It; ++It)
		{
			return *It;
		}
	}
	return nullptr;
}

void AKRGusDirector::Shout(const FText& Text) const
{
	if (AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
	{
		GameMode->ShowMessage(Text);
	}
}

void AKRGusDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds); // world time: Gus freezes when you do

	const AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>();
	if (GameMode && GameMode->IsGameOver())
	{
		Pending = EPending::None;
		WarningHoldLeft = 0.f;
		return;
	}

	// [M3.5] Count down the warning, then do the thing.
	if (Pending != EPending::None)
	{
		PendingSecondsLeft -= DeltaSeconds;
		if (PendingSecondsLeft <= 0.f)
		{
			RunPending();
		}
	}
	else if (WarningHoldLeft > 0.f)
	{
		WarningHoldLeft -= DeltaSeconds;
	}

	const UKRTuningData* T = UKRTuningData::Get(this);

	if (bAutoToss && T->TossInterval > 0.f) // [M4] separate switches
	{
		SecondsToToss -= DeltaSeconds;
		if (SecondsToToss <= 0.f)
		{
			SecondsToToss = T->TossInterval;
			if (Pending == EPending::None)
			{
				TossIngredient();
			}
			else
			{
				SecondsToToss = 5.f; // Gus is busy: try again shortly
			}
		}
	}
	if (bAutoTrips && T->TripInterval > 0.f) // [M4]
	{
		SecondsToTrip -= DeltaSeconds;
		if (SecondsToTrip <= 0.f)
		{
			SecondsToTrip = T->TripInterval;
			SecondsToToss = FMath::Max(SecondsToToss, 8.f); // don't toss right on top of a trip
			PlayEvent(TEXT("Trip"));
		}
	}
}

static EKRFoodType FoodForPart(EKRDishPart Part)
{
	switch (Part)
	{
	case EKRDishPart::ChoppedLettuce: return EKRFoodType::Lettuce;
	case EKRDishPart::ChoppedOnion:   return EKRFoodType::Onion;
	case EKRDishPart::CookedSteak:    return EKRFoodType::Steak;
	default:                          return EKRFoodType::Tomato; // tomato and tomato soup
	}
}

void AKRGusDirector::TossIngredient()
{
	if (FoodClasses.Num() == 0)
	{
		Shout(NSLOCTEXT("KitchenRush", "GusNoFood", "Gus has nothing to toss: fill Food Classes in BP_GusDirector."));
		return;
	}

	// What do the open tickets need?
	TSet<EKRFoodType> Needed;
	if (const AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
	{
		for (const FKROrder& Order : GameMode->GetOrders())
		{
			if (const FKRRecipeRow* Recipe = GameMode->FindRecipe(Order.RecipeRow))
			{
				for (EKRDishPart Part : Recipe->Parts)
				{
					Needed.Add(FoodForPart(Part));
				}
			}
		}
	}

	// Pick from the useful ingredients if there are any, otherwise from all of them.
	// GetDefaultObject() is the class's template object: we can read its settings without spawning one.
	TArray<TSubclassOf<AKRFood>> Choices;
	for (const TSubclassOf<AKRFood>& FoodClass : FoodClasses)
	{
		if (FoodClass && Needed.Contains(FoodClass->GetDefaultObject<AKRFood>()->FoodType))
		{
			Choices.Add(FoodClass);
		}
	}
	if (Choices.Num() == 0)
	{
		for (const TSubclassOf<AKRFood>& FoodClass : FoodClasses)
		{
			if (FoodClass)
			{
				Choices.Add(FoodClass);
			}
		}
	}
	if (Choices.Num() == 0)
	{
		return;
	}
	const TSubclassOf<AKRFood> Picked = Choices[FMath::RandRange(0, Choices.Num() - 1)];
	if (Pending == EPending::Event)
	{
		return; // a trip is coming: don't replace it with a toss
	}
	// [M3.5] Call it out first; Tick launches it when the countdown runs out.
	// [M3.6] A toss is just a normal HUD message (no big warning or arrow).
	Pending = EPending::Toss;
	PendingFood = Picked;
	StartWarning(FText::GetEmpty(), UKRTuningData::Get(this)->TossWarningSeconds);
	Shout(FText::Format(NSLOCTEXT("KitchenRush", "GusToss", "Gus: Heads up, chef! {0} coming your way!"),
		Picked->GetDefaultObject<AKRFood>()->DisplayName));
}

bool AKRGusDirector::PlayEvent(FName EventName)
{
	const FKRGusEvent* Event = Events.FindByPredicate([EventName](const FKRGusEvent& E) { return E.Name == EventName; });
	if (!Event)
	{
		return false;
	}
	// [M3.5] Warn first; Tick calls LaunchEventRows when the warning runs out.
	Pending = EPending::Event;
	PendingEvent = EventName;
	WarningLocation = FindOrigin(Event->OriginTag); // [M4] the HUD arrow points here
	StartWarning(Event->Shout, Event->WarningSeconds > 0.f ? Event->WarningSeconds : UKRTuningData::Get(this)->TripWarningSeconds);
	return true;
}

// [M3.5] (This was the second half of PlayEvent in M3.)
void AKRGusDirector::LaunchEventRows(const FKRGusEvent& Event)
{
	const FName EventName = Event.Name;
	const FVector Origin = FindOrigin(Event.OriginTag); // [M4] Gus, or e.g. the shelf
	for (const FKRGusLaunch& Launch : Event.Launches)
	{
		if (!Launch.ItemClass)
		{
			if (!bWarnedEmptyClass)
			{
				bWarnedEmptyClass = true;
				Shout(FText::Format(NSLOCTEXT("KitchenRush", "GusEmptyRow", "Gus event '{0}' has a row with no Item Class (BP_GusDirector)."), FText::FromName(EventName)));
			}
			continue;
		}
		if (Launch.Delay <= 0.f)
		{
			LaunchItemFrom(Launch.ItemClass, Launch.Aim, Origin);
			continue;
		}
		// World timers run on world time, so a frozen player also freezes the rest of the burst.
		FTimerHandle Handle;
		const TSubclassOf<AKRItem> ItemClass = Launch.ItemClass;
		const EKRGusAim Aim = Launch.Aim;
		GetWorldTimerManager().SetTimer(Handle, FTimerDelegate::CreateWeakLambda(this, [this, ItemClass, Aim, Origin]()
		{
			LaunchItemFrom(ItemClass, Aim, Origin);
		}), Launch.Delay, false);
	}
}

FVector AKRGusDirector::GetAimPoint(EKRGusAim Aim, const FVector& Origin) const
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return Origin + GetActorForwardVector() * 300.f;
	}
	const FVector PlayerLocation = Player->GetActorLocation(); // middle of the capsule, about 1 m up
	const FVector Chest = PlayerLocation + FVector(0.f, 0.f, 30.f);

	switch (Aim)
	{
	case EKRGusAim::AtPlayer:
		return Chest;
	case EKRGusAim::InFrontOfPlayer:
	{
		const FVector TowardGus = (Origin - PlayerLocation).GetSafeNormal2D(); // [M4] toward wherever it's thrown from
		return Chest + TowardGus * CatchPointDistance;
	}
	case EKRGusAim::NearPlayer:
	default:
	{
		const FVector2D Offset = FMath::RandPointInCircle(ScatterRadius);
		return PlayerLocation + FVector(Offset.X, Offset.Y, -80.f);
	}
	}
}

AKRItem* AKRGusDirector::LaunchItem(TSubclassOf<AKRItem> ItemClass, EKRGusAim Aim)
{
	return LaunchItemFrom(ItemClass, Aim, GetActorLocation()); // [M4]
}

// [M4] (was the body of LaunchItem)
AKRItem* AKRGusDirector::LaunchItemFrom(TSubclassOf<AKRItem> ItemClass, EKRGusAim Aim, const FVector& Origin)
{
	if (!ItemClass)
	{
		return nullptr;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);
	const FVector2D Spread = FMath::RandPointInCircle(SpawnSpread);
	const FVector Start = Origin + FVector(Spread.X, Spread.Y, FMath::FRandRange(-SpawnSpread, SpawnSpread) * 0.5f);
	const FVector Target = GetAimPoint(Aim, Origin);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AKRItem* Item = GetWorld()->SpawnActor<AKRItem>(ItemClass, Start, FRotator::ZeroRotator, Params);
	if (!Item)
	{
		return nullptr;
	}

	FVector Velocity;
	const AKRHazardProjectile* Hazard = Cast<AKRHazardProjectile>(Item);
	if (Hazard && Hazard->IsKnife())
	{
		// Knives fly fast and almost straight: aim at the target and add just enough lift for their weak gravity.
		const float Distance = FVector::Dist(Start, Target);
		const float FlightTime = Distance / FMath::Max(T->KnifeSpeed, 1.f);
		const float Gravity = -GetWorld()->GetGravityZ() * T->KnifeGravityScale;
		Velocity = (Target - Start).GetSafeNormal() * T->KnifeSpeed + FVector(0.f, 0.f, 0.5f * Gravity * FlightTime);
	}
	else if (!UGameplayStatics::SuggestProjectileVelocity_CustomArc(this, Velocity, Start, Target, 0.f, T->TossArc))
	{
		Velocity = (Target - Start).GetSafeNormal() * 600.f + FVector(0.f, 0.f, 300.f); // fallback lob
	}

	Item->OnThrown(Start, Velocity, true);
	return Item;
}

// ---------------- Debug console commands ----------------
// Press ` (the key under Esc) while playing and type one of these.

static void KRCmdToss(UWorld* World)
{
	if (AKRGusDirector* Gus = AKRGusDirector::Find(World))
	{
		Gus->TossIngredient();
	}
}

static void KRCmdTrip(UWorld* World)
{
	if (AKRGusDirector* Gus = AKRGusDirector::Find(World))
	{
		Gus->PlayEvent(TEXT("Trip"));
	}
}

static void KRCmdEvent(const TArray<FString>& Args, UWorld* World)
{
	AKRGusDirector* Gus = AKRGusDirector::Find(World);
	if (Gus && Args.Num() > 0)
	{
		Gus->PlayEvent(FName(*Args[0]));
	}
}

static void KRCmdHurt(const TArray<FString>& Args, UWorld* World)
{
	if (UKRHealthComponent* Health = UKRHealthComponent::FindPlayerHealth(World))
	{
		Health->ApplyDamage(Args.Num() > 0 ? FCString::Atof(*Args[0]) : 25.f);
	}
}

static void KRCmdGusAuto(const TArray<FString>& Args, UWorld* World)
{
	if (AKRGusDirector* Gus = AKRGusDirector::Find(World))
	{
		Gus->SetAutoEvents(Args.Num() == 0 || FCString::Atoi(*Args[0]) != 0);
	}
}

static FAutoConsoleCommandWithWorld GKRToss(TEXT("kr.Toss"), TEXT("Gus tosses you an ingredient."),
	FConsoleCommandWithWorldDelegate::CreateStatic(&KRCmdToss));
static FAutoConsoleCommandWithWorld GKRTrip(TEXT("kr.Trip"), TEXT("Gus trips (plays the Trip event)."),
	FConsoleCommandWithWorldDelegate::CreateStatic(&KRCmdTrip));
static FAutoConsoleCommandWithWorldAndArgs GKREvent(TEXT("kr.Event"), TEXT("Play a Gus event by name, e.g. kr.Event Trip"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&KRCmdEvent));
static FAutoConsoleCommandWithWorldAndArgs GKRHurt(TEXT("kr.Hurt"), TEXT("Damage the player, e.g. kr.Hurt 30"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&KRCmdHurt));
static FAutoConsoleCommandWithWorldAndArgs GKRGusAuto(TEXT("kr.GusAuto"), TEXT("kr.GusAuto 0 stops Gus's timed tosses and trips; 1 turns them back on."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&KRCmdGusAuto));
