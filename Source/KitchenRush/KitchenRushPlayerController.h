// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Flow/KRFlowTypes.h"
#include "KitchenRushPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UUserWidget;
class UKRHUDWidget;
class UKRMainMenuWidget;
class UKRPauseMenuWidget;
class UKRSettingsWidget;
class UKRResultsWidget;
class UKRNoteCardWidget;

/**
 *  Simple first person Player Controller
 *  Manages the input mapping context.
 *  Overrides the Player Camera Manager class.
 */
UCLASS(abstract, config="Game")
class KITCHENRUSH_API AKitchenRushPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	AKitchenRushPlayerController();

	// ---------------- [M4] Menus and screens ----------------
	// Every screen pauses the game and shows the mouse; closing the last one goes back to playing.

	/** The main menu, over the frozen kitchen. */
	void ShowMainMenu();

	/** Main menu buttons. */
	void PlayFromMenu();
	void ContinueFromMenu();

	/** Esc / P: open or close the pause menu (ignored while another screen is up). */
	void TogglePause();

	/** Settings on top of the main menu or pause menu; Back closes it. */
	void OpenSettings();
	void CloseSettings();

	/** One of Nana's notes. Notes queue up if one is already showing. */
	void ShowNote(const FText& Title, const FText& Body);

	/** Note card buttons: "Got it" (bSkipTutorial false) or "Skip tutorial" (true). */
	void CloseNote(bool bSkipTutorial);

	/** The Results (won) or Game Over screen. */
	void ShowResults(const FKRResults& Results);

	/** Reload the kitchen and start an act straight away (Restart act, Play again). */
	void RestartAtAct(int32 Act);

	/** Restart the act you're in (pause menu). */
	void RestartCurrentAct();

	/** Reload the kitchen and show the main menu. */
	void GoToMainMenu();

	void QuitToDesktop();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Which HUD to show */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UKRHUDWidget> HUDWidgetClass;

	/** The HUD that's on screen */
	UPROPERTY()
	TObjectPtr<UKRHUDWidget> HUDWidget;

	// ---- [M4] Screens: set each to its WBP_ in BP_FirstPersonPlayerController ----
	UPROPERTY(EditAnywhere, Category = "UI|Screens") TSubclassOf<UKRMainMenuWidget> MainMenuClass;
	UPROPERTY(EditAnywhere, Category = "UI|Screens") TSubclassOf<UKRPauseMenuWidget> PauseMenuClass;
	UPROPERTY(EditAnywhere, Category = "UI|Screens") TSubclassOf<UKRSettingsWidget> SettingsClass;
	UPROPERTY(EditAnywhere, Category = "UI|Screens") TSubclassOf<UKRResultsWidget> ResultsClass;
	UPROPERTY(EditAnywhere, Category = "UI|Screens") TSubclassOf<UKRNoteCardWidget> NoteCardClass;

	/** [M4] IA_Pause (Esc and P). Tick "Trigger when Paused" on the asset so it can also close the menu. */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

private:
	// [M4]
	struct FQueuedNote
	{
		FText Title;
		FText Body;
	};

	/** Add a screen on top, pause, show the mouse and give it keyboard focus. */
	void OpenScreen(UUserWidget* Screen, int32 ZOrder);

	/** Remove a screen; when none are left, unpause and go back to playing. */
	void CloseScreen(UUserWidget* Screen);

	/** Is any screen up? */
	bool IsAnyScreenOpen() const;

	void ShowNextNote();
	void OnPausePressed();

	UPROPERTY() TObjectPtr<UKRMainMenuWidget> MainMenu;
	UPROPERTY() TObjectPtr<UKRPauseMenuWidget> PauseMenu;
	UPROPERTY() TObjectPtr<UKRSettingsWidget> Settings;
	UPROPERTY() TObjectPtr<UKRResultsWidget> ResultsScreen;
	UPROPERTY() TObjectPtr<UKRNoteCardWidget> NoteCard;

	/** Screens currently up, bottom to top. */
	UPROPERTY() TArray<TObjectPtr<UUserWidget>> OpenScreens;

	TArray<FQueuedNote> NoteQueue;
};
