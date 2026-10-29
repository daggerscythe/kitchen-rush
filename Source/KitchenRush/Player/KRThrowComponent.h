// KRThrowComponent.h
// Hold the left mouse button to aim and release to throw it. Throwing is an action, so it plays a short burst of time like E does.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KRThrowComponent.generated.h"

class UCameraComponent;
class UInstancedStaticMeshComponent;
class UKRInteractionComponent;

UCLASS(ClassGroup = (KitchenRush), meta = (BlueprintSpawnableComponent))
class KITCHENRUSH_API UKRThrowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKRThrowComponent();

	/** The character calls this in BeginPlay. ArcDots draws the aim arc (one instance per dot). */
	void Setup(UCameraComponent* InCamera, UKRInteractionComponent* InInteraction, UInstancedStaticMeshComponent* InArcDots);

	/** LMB pressed: start showing the arc (only if you're holding something). */
	void StartAim();

	/** LMB released: throw. */
	void ReleaseThrow();

	bool IsAiming() const { return bAiming; }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	/** How many seconds of flight the arc preview shows. */
	UPROPERTY(EditAnywhere, Category = "Throw")
	float ArcSeconds = 1.5f;

	/** Dots per second of flight. */
	UPROPERTY(EditAnywhere, Category = "Throw")
	float DotsPerSecond = 20.f;

	/** Dot size (the engine sphere is 100 cm, so 0.03 = 3 cm). The landing dot is drawn 2.5x bigger. */
	UPROPERTY(EditAnywhere, Category = "Throw")
	float DotScale = 0.03f;

private:
	FVector GetThrowVelocity() const;
	void UpdateArc();
	void ClearArc();

	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY()
	TObjectPtr<UKRInteractionComponent> Interaction;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> ArcDots;

	bool bAiming = false;
};
