// KRPass.cpp

#include "Orders/KRPass.h"
#include "Cooking/KRPlate.h"
#include "Interaction/KRInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "KitchenRushGameMode.h"

AKRPass::AKRPass()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->InitBoxExtent(FVector(25.f, 70.f, 30.f)); // half-sizes in cm: scale it to fill the window
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	Box->ShapeColor = FColor::Green;
}

FText AKRPass::GetInteractPrompt(const AKRItem* HeldItem) const
{
	const AKRPlate* Plate = Cast<AKRPlate>(HeldItem);
	if (!Plate || Plate->IsEmpty())
	{
		return FText::GetEmpty();
	}
	const AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>();
	if (!GameMode || GameMode->GetOrders().Num() == 0)
	{
		return NSLOCTEXT("KitchenRush", "NoOrders", "No orders yet");
	}
	return FText::Format(NSLOCTEXT("KitchenRush", "ServePrompt", "Serve {0}"), Plate->GetItemName());
}

bool AKRPass::Interact(UKRInteractionComponent* Interactor)
{
	AKRPlate* Plate = Interactor ? Cast<AKRPlate>(Interactor->GetHeldItem()) : nullptr;
	if (!Plate || Plate->IsEmpty())
	{
		return false;
	}
	AKitchenRushGameMode* GameMode = GetWorld()->GetAuthGameMode<AKitchenRushGameMode>();
	if (!GameMode || !GameMode->ServeDish(Plate))
	{
		return false;
	}
	// Served: the plate (and the food on it) leaves through the window.
	Interactor->ReleaseHeldItem()->Destroy();
	return true;
}
