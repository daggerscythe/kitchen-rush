// KRInteractionComponent.cpp

#include "Interaction/KRInteractionComponent.h"
#include "Interaction/KRInteractable.h"
#include "Interaction/KRItem.h"
#include "Time/KRTimeSubsystem.h"
#include "Cooking/KRTuningData.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UKRInteractionComponent::UKRInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKRInteractionComponent::Setup(UCameraComponent* InCamera, USceneComponent* InHoldPoint)
{
	Camera = InCamera;
	HoldPoint = InHoldPoint;
}

AKRItem* UKRInteractionComponent::GetHeldItem() const
{
	return HeldItem;
}

AKRItem* UKRInteractionComponent::ReleaseHeldItem()
{
	AKRItem* Item = HeldItem;
	HeldItem = nullptr;
	return Item;
}

void UKRInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus();
}

void UKRInteractionComponent::UpdateFocus()
{
	FocusedActor.Reset();
	bCanPlace = false;
	Prompt = FText::GetEmpty();

	if (!Camera)
	{
		return;
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * Reach;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(KRInteractTrace), false, GetOwner());
	if (HeldItem)
	{
		Params.AddIgnoredActor(HeldItem.Get());
	}

	// [M3] Case 0: catching. Flying things are small and fast, so sweep a fat sphere instead of a thin
	// line (aim assist), out to the catch range. If it finds something catchable, that wins.
	{
		const UKRTuningData* T = UKRTuningData::Get(this);
		const FVector CatchEnd = Start + Camera->GetForwardVector() * T->CatchRange;
		FHitResult CatchHit;
		if (GetWorld()->SweepSingleByChannel(CatchHit, Start, CatchEnd, FQuat::Identity, ECC_Visibility,
			FCollisionShape::MakeSphere(T->CatchAssistRadius), Params))
		{
			IKRInteractable* Flying = Cast<IKRInteractable>(CatchHit.GetActor());
			if (Flying && Flying->IsCatchable())
			{
				const FText CatchPrompt = Flying->GetInteractPrompt(HeldItem.Get());
				if (!CatchPrompt.IsEmpty())
				{
					FocusedActor = CatchHit.GetActor();
					Prompt = CatchPrompt;
					return;
				}
			}
		}
	}

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return;
	}

	// Case 1: something interactable with something to say.
	if (IKRInteractable* Target = Cast<IKRInteractable>(Hit.GetActor()))
	{
		const FText TargetPrompt = Target->GetInteractPrompt(HeldItem.Get());
		if (!TargetPrompt.IsEmpty())
		{
			FocusedActor = Hit.GetActor();
			Prompt = TargetPrompt;
			return;
		}
	}

	// Case 2: holding something and looking at a flat-enough surface.
	if (HeldItem && Hit.ImpactNormal.Z >= MinPlaceSurfaceUp)
	{
		bCanPlace = true;
		PlaceLocation = Hit.ImpactPoint;
		Prompt = FText::Format(NSLOCTEXT("KitchenRush", "PlacePrompt", "Place {0}"), HeldItem->GetItemName());
	}
}

void UKRInteractionComponent::Interact()
{
	bool bDidSomething = false;

	if (AActor* Target = FocusedActor.Get())
	{
		if (IKRInteractable* Interactable = Cast<IKRInteractable>(Target))
		{
			bDidSomething = Interactable->Interact(this);
		}
	}
	else if (HeldItem && bCanPlace)
	{
		PlaceHeldItem();
		bDidSomething = true;
	}

	if (bDidSomething)
	{
		if (UKRTimeSubsystem* Time = GetWorld()->GetSubsystem<UKRTimeSubsystem>())
		{
			Time->TriggerActionBurst();
		}
		UpdateFocus();
	}
}

bool UKRInteractionComponent::TryPickUp(AKRItem* Item)
{
	if (!Item || HeldItem || !HoldPoint)
	{
		return false;
	}
	HeldItem = Item;
	Item->OnPickedUp(HoldPoint);
	return true;
}

void UKRInteractionComponent::PlaceHeldItem()
{
	AKRItem* Item = HeldItem;
	HeldItem = nullptr;

	const float Yaw = GetOwner()->GetActorRotation().Yaw;
	Item->OnPlaced(PlaceLocation + FVector(0.f, 0.f, 1.f), FRotator(0.f, Yaw, 0.f));
}
