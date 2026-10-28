// KRPan.cpp

#include "Cooking/KRPan.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRPlate.h"
#include "Cooking/KRTuningData.h"
#include "Interaction/KRInteractionComponent.h"

AKRPan::AKRPan()
{
	DisplayName = NSLOCTEXT("KitchenRush", "PanName", "Pan");

	FoodSlot = CreateDefaultSubobject<USceneComponent>(TEXT("Food Slot"));
	FoodSlot->SetupAttachment(Mesh);
	FoodSlot->SetRelativeLocation(FVector(0.f, 0.f, 1.5f));
}

FText AKRPan::GetInteractPrompt(const AKRItem* HeldItem) const
{
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
		return FText::Format(NSLOCTEXT("KitchenRush", "TakeFromPan", "Take {0}"), Steak->GetItemName());
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
			HeldFood->OnStored(FoodSlot, FVector::ZeroVector);
			Steak = HeldFood;
			return true;
		}
		return Super::Interact(Interactor);
	}

	AKRFood* Taken = Steak;
	if (!Held)
	{
		Steak = nullptr;
		return Interactor->TryPickUp(Taken);
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

void AKRPan::Cook(float WorldSeconds)
{
	if (Steak)
	{
		Steak->AddCookTime(WorldSeconds);
		// M3: a steak left 3 s past burnt starts a grease fire here.
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

void AKRPan::Destroyed()
{
	if (Steak)
	{
		Steak->Destroy();
	}
	Super::Destroyed();
}
