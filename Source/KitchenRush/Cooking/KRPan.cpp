// KRPan.cpp

#include "Cooking/KRPan.h"
#include "Audio/KRAudioSubsystem.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "Components/StaticMeshComponent.h" // (Mesh as a scene component for the sizzle)
#include "Interaction/KRInteractionComponent.h"
#include "Hazards/KRHazardZone.h"
#include "Engine/World.h"
#include "KitchenRushGameMode.h"

AKRPan::AKRPan()
{
	DisplayName = NSLOCTEXT("KitchenRush", "PanName", "Pan");

	FoodSlot = CreateDefaultSubobject<USceneComponent>(TEXT("Food Slot"));
	FoodSlot->SetupAttachment(Mesh);
	FoodSlot->SetRelativeLocation(FVector(0.f, 0.f, 1.5f));
}

bool AKRPan::CanReceiveItem(const AKRItem* Item) const
{
	const AKRFood* Food = Cast<AKRFood>(Item);
	return !Steak && Food && Food->CanBeCooked();
}

bool AKRPan::ReceiveThrownItem(AKRItem* Item)
{
	if (!CanReceiveItem(Item))
	{
		return false;
	}
	AddSteak(Cast<AKRFood>(Item));
	return true;
}

void AKRPan::AddSteak(AKRFood* Food)
{
	Food->OnStored(FoodSlot, FVector::ZeroVector);
	Steak = Food;
}

FText AKRPan::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (IsInFlight())
	{
		return GetCatchPrompt(HeldItem);
	}
	if (IsHeld())
	{
		return FText::GetEmpty();
	}
	if (!Steak)
	{
		const AKRFood* HeldFood = Cast<AKRFood>(HeldItem);
		if (HeldFood && HeldFood->CanBeCooked())
		{
			return FText::Format(NSLOCTEXT("KitchenRush", "PutInPan", "Put {0} in pan"), HeldFood->GetItemName());
		}
		return Super::GetInteractPrompt(HeldItem);
	}
	if (!HeldItem)
	{
		return bAimingAtSteak
			? FText::Format(NSLOCTEXT("KitchenRush", "TakeFromPan", "Take {0}"), Steak->GetItemName())
			: FText::Format(NSLOCTEXT("KitchenRush", "PickUpPanWith", "Pick up Pan with {0}"), Steak->GetItemName());
	}
	if (const AKRPlate* Plate = Cast<AKRPlate>(HeldItem))
	{
		if (Plate->CanAcceptFood(Steak))
		{
			return FText::Format(NSLOCTEXT("KitchenRush", "PanToPlate", "Put {0} on {1}"), Steak->GetItemName(), Plate->DisplayName);
		}
	}
	return FText::GetEmpty();
}

bool AKRPan::Interact(UKRInteractionComponent* Interactor)
{
	if (IsInFlight())
	{
		return CatchBy(Interactor);
	}
	if (!Interactor || IsHeld())
	{
		return false;
	}
	AKRItem* Held = Interactor->GetHeldItem();

	if (!Steak)
	{
		AKRFood* HeldFood = Cast<AKRFood>(Held);
		if (HeldFood && HeldFood->CanBeCooked())
		{
			Interactor->ReleaseHeldItem();
			AddSteak(HeldFood);
			return true;
		}
		return Super::Interact(Interactor);
	}

	AKRFood* Taken = Steak;
	if (!Held)
	{
		if (bAimingAtSteak)
		{
			Steak = nullptr;
			return Interactor->TryPickUp(Taken); // just the steak
		}
		return Super::Interact(Interactor); // the pan, with the steak in it
	}
	if (AKRPlate* Plate = Cast<AKRPlate>(Held))
	{
		if (Plate->CanAcceptFood(Taken))
		{
			Steak = nullptr;
			return Plate->AddFood(Taken);
		}
	}
	return false;
}

void AKRPan::UpdateAim(const FVector& Point)
{
	// The steak's outline seen from above (plus 2 cm); a point inside it is "on the steak".
	bAimingAtSteak = Steak && Steak->GetComponentsBoundingBox(true).ExpandBy(2.f).IsInsideXY(Point);
}

void AKRPan::Cook(float WorldSeconds)
{
	if (Steak)
	{
		const UKRTuningData* T = UKRTuningData::Get(this);
		const float Before = Steak->GetCookSeconds(); // [M4.3]
		Steak->AddCookTime(WorldSeconds);
		// Ding: it just reached the perfect window.
		if (Before < T->SteakCookedAt && Steak->GetCookSeconds() >= T->SteakCookedAt)
		{
			UKRAudioSubsystem::PlayAt(this, EKRSound::CookDone, GetActorLocation());
		}
		// A steak left GreaseFireDelay seconds past burnt catches fire.
		if (Steak->GetCookSeconds() >= T->SteakBurntAt + T->GreaseFireDelay)
		{
			StartGreaseFire();
		}
	}
}

void AKRPan::StartGreaseFire()
{
	const UKRTuningData* T = UKRTuningData::Get(this);

	// The steak is ash now.
	Steak->Destroy();
	Steak = nullptr;
	UKRAudioSubsystem::PlayAt(this, EKRSound::FireFlare, GetActorLocation()); // [M4.3] whoomph

	UClass* FireClass = GreaseFireClass ? GreaseFireClass.Get() : AKRHazardZone::StaticClass();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AKRHazardZone* Fire = GetWorld()->SpawnActor<AKRHazardZone>(FireClass, GetActorLocation(), FRotator::ZeroRotator, Params))
	{
		Fire->Start(T->GreaseFireSeconds, T->GreaseFireDamagePerSecond, T->GreaseFireRadius, true);
	}
	if (AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>())
	{
		GameMode->ShowMessage(NSLOCTEXT("KitchenRush", "GreaseFire", "GREASE FIRE! Stay back, that burner is out for a while."));
	}
}

void AKRPan::UpdateMeter()
{
	if (!Steak)
	{
		SetMeter(false, 0.f, FLinearColor::White, FText::GetEmpty());
		return;
	}
	const UKRTuningData* T = UKRTuningData::Get(this);
	const float Seconds = Steak->GetCookSeconds();

	FLinearColor Color(0.85f, 0.85f, 0.85f);                                             // raw: grey
	if (Seconds >= T->SteakBurntAt)            Color = FLinearColor(0.9f, 0.2f, 0.2f);   // burnt: red
	else if (Seconds > T->SteakPerfectUntil)   Color = FLinearColor(1.f, 0.6f, 0.2f);    // well done: orange
	else if (Seconds >= T->SteakCookedAt)      Color = FLinearColor(0.3f, 0.85f, 0.4f);  // perfect: green

	SetMeter(true, FMath::Clamp(Seconds / T->SteakBurntAt, 0.f, 1.f), Color, Steak->GetItemName());
}

// Sizzle while a steak is in the pan on a burner.
void AKRPan::UpdateCookSound()
{
	UKRAudioSubsystem::UpdateLoop(this, CookLoop, EKRSound::Sizzle, Steak != nullptr && IsOnBurner(), Mesh);
}

void AKRPan::Destroyed()
{
	if (Steak)
	{
		Steak->Destroy();
	}
	Super::Destroyed();
}
