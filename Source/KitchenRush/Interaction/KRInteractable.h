// KRInteractable.h

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KRInteractable.generated.h"

class AKRItem;
class UKRInteractionComponent;

UINTERFACE(MinimalAPI)
class UKRInteractable : public UInterface
{
	GENERATED_BODY()
};

class KITCHENRUSH_API IKRInteractable
{
	GENERATED_BODY()

public:
	/**
	 * Text shown under the crosshair
	 * HeldItem is what the player is carrying (nullptr = empty hands)
	 * Return empty text if there's nothing to do right now
	 */
	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const = 0;

	/** The player pressed E while looking at this, true if something happened */
	virtual bool Interact(UKRInteractionComponent* Interactor) = 0;
};
