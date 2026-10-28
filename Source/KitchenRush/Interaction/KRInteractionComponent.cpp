// KRInteractionComponent.cpp

#include "Interaction/KRInteractionComponent.h"
#include "Interaction/KRInteractable.h"
#include "Interaction/KRItem.h"
#include "Time/KRTimeSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UKRInteractionComponent::UKRInteractionComponent()
{
	// Components only tick if you ask them to
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

	// A line from the camera straight ahead, Reach cm long
	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * Reach;

	// Ignore ourselves and the thing in our hands
	FCollisionQueryParams Params(SCENE_QUERY_STAT(KRInteractTrace), false, GetOwner());
	if (HeldItem)
	{
		Params.AddIgnoredActor(HeldItem.Get());
	}

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		return; // looking at nothing within reach
	}

	// Case 1: we're looking at something interactable that has something to say
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

	// Case 2: we're holding something and looking at a flat surface: offer to place it
	if (HeldItem && Hit.ImpactNormal.Z >= MinPlaceSurfaceUp)
	{
		bCanPlace = true;
		PlaceLocation = Hit.ImpactPoint;
		Prompt = FText::Format(NSLOCTEXT("KitchenRush", "PlacePrompt", "Place {0}"), HeldItem->DisplayName);
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
		// Every action costs a little time
		if (UKRTimeSubsystem* Time = GetWorld()->GetSubsystem<UKRTimeSubsystem>())
		{
			Time->TriggerActionBurst();
		}
		UpdateFocus(); // refresh the prompt right away
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

	// Lift 1 cm so physics doesn't start the item inside the counter; Face the same way as the player
	const float Yaw = GetOwner()->GetActorRotation().Yaw;
	Item->OnPlaced(PlaceLocation + FVector(0.f, 0.f, 1.f), FRotator(0.f, Yaw, 0.f));
}
