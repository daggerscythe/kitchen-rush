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
	virtual void BeginPlay() override;

	/** Something physical entered the box: if it's a thrown plate, serve it. */
	UFUNCTION()
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** Blocks the Visibility channel (look-at trace) and [M3.5] overlaps physics objects (thrown plates). Nothing bumps into it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pass")
	TObjectPtr<UBoxComponent> Box;
};
