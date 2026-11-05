// KRProfileSubsystem.cpp

#include "Flow/KRProfileSubsystem.h"
#include "Flow/KRSaveGame.h"
#include "Engine/GameInstance.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

static const TCHAR* GSlotName = TEXT("KitchenRush");

void UKRProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (UGameplayStatics::DoesSaveGameExist(GSlotName, 0))
	{
		Data = Cast<UKRSaveGame>(UGameplayStatics::LoadGameFromSlot(GSlotName, 0));
	}
	if (!Data)
	{
		Data = Cast<UKRSaveGame>(UGameplayStatics::CreateSaveGameObject(UKRSaveGame::StaticClass()));
	}

	// First run: start from the current Application Scale (UI size).
	if (!Data->bSettingsInitialised)
	{
		Data->UIScale = GetDefault<UUserInterfaceSettings>()->ApplicationScale;
		Data->MouseSensitivity = 1.f;
		Data->bSettingsInitialised = true;
		Save();
	}
}

UKRProfileSubsystem* UKRProfileSubsystem::Get(const UObject* WorldContext)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext);
	return GameInstance ? GameInstance->GetSubsystem<UKRProfileSubsystem>() : nullptr;
}

void UKRProfileSubsystem::Save()
{
	UGameplayStatics::SaveGameToSlot(Data, GSlotName, 0);
}

int32 UKRProfileSubsystem::GetHighScore() const { return Data->HighScore; }
int32 UKRProfileSubsystem::GetLastActReached() const { return Data->LastActReached; }
float UKRProfileSubsystem::GetMouseSensitivity() const { return Data->MouseSensitivity; }
float UKRProfileSubsystem::GetUIScale() const { return Data->UIScale; }
float UKRProfileSubsystem::GetMasterVolume() const { return Data->MasterVolume; }
float UKRProfileSubsystem::GetMusicVolume() const { return Data->MusicVolume; }

void UKRProfileSubsystem::RecordAct(int32 Act)
{
	if (Act > Data->LastActReached)
	{
		Data->LastActReached = FMath::Clamp(Act, 1, 3);
		Save();
	}
}

bool UKRProfileSubsystem::RecordTips(int32 Tips, bool bWon)
{
	const bool bNewBest = Tips > Data->HighScore;
	if (bNewBest)
	{
		Data->HighScore = Tips;
	}
	Data->bHasWon |= bWon;
	Save();
	return bNewBest;
}

void UKRProfileSubsystem::SetMouseSensitivity(float Value)
{
	Data->MouseSensitivity = FMath::Clamp(Value, 0.1f, 5.f);
	Save();
}

void UKRProfileSubsystem::SetUIScale(float Value)
{
	Data->UIScale = FMath::Clamp(Value, 0.5f, 3.f);
	GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = Data->UIScale;
	Save();
}

void UKRProfileSubsystem::SetMasterVolume(float Value)
{
	Data->MasterVolume = FMath::Clamp(Value, 0.f, 1.f);
	Save();
}

void UKRProfileSubsystem::SetMusicVolume(float Value)
{
	Data->MusicVolume = FMath::Clamp(Value, 0.f, 1.f);
	Save();
}

void UKRProfileSubsystem::ApplySettings(UWorld* World) const
{
	// UI scale is an engine-wide setting (the same one as Project Settings > User Interface > Application Scale).
	GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = Data->UIScale;
}
