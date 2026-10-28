// KRTuningData.cpp 

#include "Cooking/KRTuningData.h"
#include "Engine/World.h"
#include "KitchenRushGameMode.h"

const UKRTuningData* UKRTuningData::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (World)
	{
		if (const AKitchenRushGameMode* GameMode = World->GetAuthGameMode<AKitchenRushGameMode>())
		{
			if (const UKRTuningData* Tuning = GameMode->GetTuning())
			{
				return Tuning;
			}
		}
	}
	// GetDefault returns the C++ default values.
	return GetDefault<UKRTuningData>();
}
