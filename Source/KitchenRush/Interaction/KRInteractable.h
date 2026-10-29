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

	// ---- [M3] Throwing and catching. These have default bodies, so classes only override what they use. ----

	/** True while this is flying through the air and can be caught (thrown or tossed items, knives). */
	virtual bool IsCatchable() const { return false; }

	/** Could Item land in / on this right now? (A plate takes food, the pan takes a steak...) */
	virtual bool CanReceiveItem(const AKRItem* Item) const { return false; }

	/** Item was thrown into this, or caught while the player held this. Return true if it was taken. */
	virtual bool ReceiveThrownItem(AKRItem* Item) { return false; }

	/** A thrown item hit this but wasn't taken (e.g. a pan bouncing off the pot). */
	virtual void OnHitByThrownItem(AKRItem* Item) {}

	/** [M3.7] Which trick-shot place this is (KRTrick::Board, Plate, Pan, Pot), or 0 if it isn't one. */
	virtual uint8 GetTrickShotFlag() const { return 0; }
};
