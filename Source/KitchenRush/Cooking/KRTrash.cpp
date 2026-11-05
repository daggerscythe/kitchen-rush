// KRTrash.cpp

#include "Cooking/KRTrash.h"
#include "Audio/KRAudioSubsystem.h"
#include "Cooking/KRFood.h"
#include "Cooking/KRPlate.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Hazards/KRHazardProjectile.h"
#include "Hazards/KRShards.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"

AKRTrash::AKRTrash()
{
	PrimaryActorTick.bCanEverTick = true; // for the lid
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// The lid is a second mesh on the same actor, so looking at it counts as looking at the bin.
	// (Named "Hinged Lid" / HingedLid so it can't clash with a "Lid" you may have added in BP_Trash before.)
	HingedLid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hinged Lid"));
	HingedLid->SetupAttachment(Mesh);
	HingedLid->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
}

void AKRTrash::BeginPlay()
{
	Super::BeginPlay();
	LidClosedRotation = HingedLid->GetRelativeRotation();
}

bool AKRTrash::IsBinnable(const AKRItem* Item)
{
	return Cast<AKRFood>(Item) || Cast<AKRHazardProjectile>(Item) || Cast<AKRShards>(Item);
}

void AKRTrash::PopLidOpen()
{
	UKRAudioSubsystem::PlayAt(this, EKRSound::TrashDump, GetActorLocation()); // (only called when something goes in)
	KeepOpenUntil = FPlatformTime::Seconds() + 0.8;
}

void AKRTrash::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Open while the player is looking at the bin with something it would take, or just after something went in.
	bool bWantOpen = FPlatformTime::Seconds() < KeepOpenUntil;
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (const UKRInteractionComponent* Interaction = Player->FindComponentByClass<UKRInteractionComponent>())
		{
			bWantOpen |= Interaction->GetFocusedActor() == this;
		}
	}

	// FApp::GetDeltaTime() is the real frame time (DeltaSeconds here is slowed-down world time).
	LidOpenAmount = FMath::FInterpTo(LidOpenAmount, bWantOpen ? 1.f : 0.f, FApp::GetDeltaTime(), LidSpeed);
	HingedLid->SetRelativeRotation(LidClosedRotation + LidOpenRotation * LidOpenAmount);
}

FText AKRTrash::GetInteractPrompt(const AKRItem* HeldItem) const
{
	if (const AKRPlate* Plate = Cast<AKRPlate>(HeldItem))
	{
		if (!Plate->IsEmpty())
		{
			return FText::Format(NSLOCTEXT("KitchenRush", "EmptyPlatePrompt", "Empty {0}"), Plate->DisplayName);
		}
		return FText::GetEmpty();
	}
	if (IsBinnable(HeldItem)) // hot sauce too, and broken pieces
	{
		return FText::Format(NSLOCTEXT("KitchenRush", "TrashFood", "Throw away {0}"), HeldItem->GetItemName());
	}
	return FText::GetEmpty();
}

bool AKRTrash::CanReceiveItem(const AKRItem* Item) const
{
	return IsBinnable(Item); // broken pieces too
}

bool AKRTrash::ReceiveThrownItem(AKRItem* Item)
{
	if (!CanReceiveItem(Item))
	{
		return false;
	}
	Item->Destroy();
	PopLidOpen();
	return true;
}

bool AKRTrash::Interact(UKRInteractionComponent* Interactor)
{
	if (!Interactor)
	{
		return false;
	}
	AKRItem* Held = Interactor->GetHeldItem();
	if (AKRPlate* Plate = Cast<AKRPlate>(Held))
	{
		if (Plate->IsEmpty())
		{
			return false;
		}
		Plate->ClearContents(); // the plate stays in your hands
		PopLidOpen();
		return true;
	}
	if (IsBinnable(Held)) // hot sauce too, and broken pieces
	{
		Interactor->ReleaseHeldItem()->Destroy();
		PopLidOpen();
		return true;
	}
	return false;
}
