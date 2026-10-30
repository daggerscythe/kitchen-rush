// KRPot.cpp 

#include "Cooking/KRPot.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "KitchenRushGameMode.h"
#include "Hazards/KRHazardZone.h"

AKRPot::AKRPot()
{
	DisplayName = NSLOCTEXT("KitchenRush", "PotName", "Pot");

	SoupSurface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Soup Surface"));
	SoupSurface->SetupAttachment(Mesh);
	SoupSurface->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SoupSurface->SetRelativeLocation(FVector(0.f, 0.f, SurfaceEmptyZ));
	SoupSurface->SetVisibility(false);
}

bool AKRPot::IsSoupReady() const
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	return TomatoCount >= T->SoupTomatoesNeeded && SimmerSeconds >= T->SoupReadyAt;
}

static bool IsChoppedTomato(const AKRItem* Item)
{
	const AKRFood* Food = Cast<AKRFood>(Item);
	return Food && Food->FoodType == EKRFoodType::Tomato && Food->IsChopped();
}

// [M3]
bool AKRPot::CanReceiveItem(const AKRItem* Item) const
{
	return IsChoppedTomato(Item) && TomatoCount < UKRTuningData::Get(this)->SoupTomatoesNeeded;
}

// [M3]
bool AKRPot::ReceiveThrownItem(AKRItem* Item)
{
	if (!CanReceiveItem(Item))
	{
		return false;
	}
	AddTomato(Item);
	return true;
}

// [M3]
void AKRPot::AddTomato(AKRItem* Tomato)
{
	// [M3.7] The soup inherits each tomato's tally (the tomato itself is about to be destroyed).
	if (const AKRFood* Food = Cast<AKRFood>(Tomato))
	{
		const FKRPlatedPart Part = Food->GetPlatedPart();
		TomatoTally.TrickShots += Part.TrickShots;
		TomatoTally.GusAssists += Part.GusAssists;
		TomatoTally.GroundTouches += Part.GroundTouches;
	}
	Tomato->Destroy(); // the tomato goes into the soup
	++TomatoCount;
	UpdateSurface();
}

// [M4]
bool AKRPot::IsSimmering() const
{
	return !IsHeld() && TomatoCount >= UKRTuningData::Get(this)->SoupTomatoesNeeded && IsOnBurner();
}

// [M4]
void AKRPot::FastForwardToBoilOver(float SecondsLeft)
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	SimmerSeconds = FMath::Max(SimmerSeconds, T->SoupBoilOverAt - SecondsLeft);
}

FText AKRPot::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (IsInFlight()) // [M3]
	{
		return GetCatchPrompt(HeldItem);
	}
	if (IsHeld())
	{
		return FText::GetEmpty();
	}
	const UKRTuningData* T = UKRTuningData::Get(this);

	if (IsChoppedTomato(HeldItem) && TomatoCount < T->SoupTomatoesNeeded)
	{
		return FText::Format(NSLOCTEXT("KitchenRush", "AddTomato", "Add to pot ({0}/{1})"), TomatoCount + 1, T->SoupTomatoesNeeded);
	}
	if (const AKRPlate* Bowl = Cast<AKRPlate>(HeldItem))
	{
		if (Bowl->CanAcceptSoup() && IsSoupReady())
		{
			return NSLOCTEXT("KitchenRush", "Ladle", "Ladle soup into bowl");
		}
		return FText::GetEmpty();
	}
	return Super::GetInteractPrompt(HeldItem);
}

bool AKRPot::Interact(UKRInteractionComponent* Interactor)
{
	if (IsInFlight()) // [M3]
	{
		return CatchBy(Interactor);
	}
	if (!Interactor || IsHeld())
	{
		return false;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);
	AKRItem* Held = Interactor->GetHeldItem();

	if (IsChoppedTomato(Held) && TomatoCount < T->SoupTomatoesNeeded)
	{
		AddTomato(Interactor->ReleaseHeldItem());
		return true;
	}
	if (AKRPlate* Bowl = Cast<AKRPlate>(Held))
	{
		if (Bowl->CanAcceptSoup() && IsSoupReady())
		{
			Bowl->AddSoup(SimmerSeconds < T->SoupBoilWarnAt, TomatoTally); // [M3.7] the tomatoes' tally
			ResetPot();
			return true;
		}
		return false;
	}
	return Super::Interact(Interactor);
}

void AKRPot::Cook(float WorldSeconds)
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	if (TomatoCount < T->SoupTomatoesNeeded)
	{
		return;
	}
	SimmerSeconds += WorldSeconds;
	if (SimmerSeconds >= T->SoupBoilOverAt)
	{
		BoilOver();
	}
}

// [M3]
void AKRPot::BoilOver()
{
	const UKRTuningData* T = UKRTuningData::Get(this);

	UClass* ZoneClass = SpillClass ? SpillClass.Get() : AKRHazardZone::StaticClass();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AKRHazardZone* Spill = GetWorld()->SpawnActor<AKRHazardZone>(ZoneClass, GetActorLocation(), FRotator::ZeroRotator, Params))
	{
		Spill->Start(T->SpillSeconds, T->SpillDamagePerSecond, T->SpillRadius, false);
	}
	if (AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
	{
		GameMode->ShowMessage(NSLOCTEXT("KitchenRush", "BoiledOver", "The soup boiled over! Scalding spill, keep clear."));
	}
	ResetPot();
}

void AKRPot::UpdateMeter()
{
	const UKRTuningData* T = UKRTuningData::Get(this);
	if (TomatoCount == 0)
	{
		SetMeter(false, 0.f, FLinearColor::White, FText::GetEmpty());
		return;
	}
	if (TomatoCount < T->SoupTomatoesNeeded)
	{
		SetMeter(true, float(TomatoCount) / T->SoupTomatoesNeeded, FLinearColor(0.85f, 0.85f, 0.85f),
			FText::Format(NSLOCTEXT("KitchenRush", "PotTomatoes", "Tomatoes {0}/{1}"), TomatoCount, T->SoupTomatoesNeeded));
		return;
	}

	const float Percent = FMath::Clamp(SimmerSeconds / T->SoupBoilOverAt, 0.f, 1.f);
	if (SimmerSeconds < T->SoupReadyAt)
	{
		SetMeter(true, Percent, FLinearColor(0.85f, 0.85f, 0.85f), NSLOCTEXT("KitchenRush", "Simmering", "Simmering"));
	}
	else if (SimmerSeconds < T->SoupBoilWarnAt)
	{
		SetMeter(true, Percent, FLinearColor(0.3f, 0.85f, 0.4f), NSLOCTEXT("KitchenRush", "SoupReady", "Soup ready!"));
	}
	else
	{
		SetMeter(true, Percent, FLinearColor(0.9f, 0.2f, 0.2f), NSLOCTEXT("KitchenRush", "BoilingOver", "Boiling over!"));
	}
}

void AKRPot::ResetPot()
{
	TomatoCount = 0;
	TomatoTally = FKRPlatedPart(); // [M3.7]
	SimmerSeconds = 0.f;
	UpdateSurface();
}

void AKRPot::UpdateSurface()
{
	const int32 Needed = UKRTuningData::Get(this)->SoupTomatoesNeeded;
	SoupSurface->SetVisibility(TomatoCount > 0);
	const float Fill = FMath::Clamp(float(TomatoCount) / Needed, 0.f, 1.f);
	SoupSurface->SetRelativeLocation(FVector(0.f, 0.f, FMath::Lerp(SurfaceEmptyZ, SurfaceFullZ, Fill)));
}
