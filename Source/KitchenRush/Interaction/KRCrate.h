// KRCrate.h
// A crate that hands you a new item every time you press E on it

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/KRInteractable.h"
#include "KRCrate.generated.h"

class UStaticMeshComponent;

UCLASS()
class KITCHENRUSH_API AKRCrate : public AActor, public IKRInteractable
{
	GENERATED_BODY()

public:
	AKRCrate();

	// ---- IKRInteractable ----
	virtual FText GetInteractPrompt(const AKRItem* HeldItem) const override;
	virtual bool Interact(UKRInteractionComponent* Interactor) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crate")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Which item Blueprint this crate gives out */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crate")
	TSubclassOf<AKRItem> ItemClass;
};
