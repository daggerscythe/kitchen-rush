// KRPass.h
// An invisible box you place inside the pass window, serves it to the best-matching ticket.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/KRInteractable.h"
#include "KRPass.generated.h"

class UBoxComponent;

UCLASS()
class KITCHENRUSH_API AKRPass : public AActor, public IKRInteractable
{
	GENERATED_BODY()

public:
	AKRPass();

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

protected:
	/** Only blocks Visibility channel, so it catches look-at trace but nothing physical */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pass")
	TObjectPtr<UBoxComponent> Box;
};
