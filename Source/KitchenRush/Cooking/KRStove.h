// KRStove.h 
// The stove marks where the two burners are.
// A pan or pot sitting close to a burner marker cooks.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KRStove.generated.h"

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class KITCHENRUSH_API AKRStove : public AActor
{
	GENERATED_BODY()

public:
	AKRStove();

	/** True if Location is within Radius (sideways) and Height (up/down) of any burner in the level. */
	static bool IsNearBurner(UWorld* World, const FVector& Location, float Radius, float Height);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stove")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stove")
	TObjectPtr<USphereComponent> BurnerA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stove")
	TObjectPtr<USphereComponent> BurnerB;
};
