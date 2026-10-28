// KRTrash.h
// Throw away food, or empty the plate/bowl in your hands.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/KRInteractable.h"
#include "KRTrash.generated.h"

class UStaticMeshComponent;

UCLASS()
class KITCHENRUSH_API AKRTrash : public AActor, public IKRInteractable
{
	GENERATED_BODY()

public:
	AKRTrash();

	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trash")
	TObjectPtr<UStaticMeshComponent> Mesh;
};
