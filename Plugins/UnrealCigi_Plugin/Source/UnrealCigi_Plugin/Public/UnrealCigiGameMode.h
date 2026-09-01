//Copyright SimBlocks LLC 2016-2026
/**
 * @file unrealcigiGameMode.h
 * @brief Defines the AunrealcigiGameMode class for SimBlocks UnrealCIGI plugin game mode integration.
 *
 * This header provides:
 * - The AunrealcigiGameMode class, derived from AGameModeBase, which is essential for enabling CIGI packet and message handling in Unreal.
 * - Game mode lifecycle overrides for starting, stopping, and ticking the CIGI image generator.
 * - Integration with the CigiView pawn and CigiHUD for view and symbol surface control.
 *
 * Usage:
 * - Set AunrealcigiGameMode as the active game mode to enable CIGI communication and simulation features.
 * - The Tick event manages all CIGI packet send/receive operations.
 * - BeginPlay and EndPlay handle IG startup and shutdown.
 */

#pragma once

#pragma warning(disable : 5103)

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "unrealcigiGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCigiGameMode, Log, All)

/**
 * @class AunrealcigiGameMode
 * @brief Game mode class required for UnrealCIGI plugin functionality.
 *
 * Handles CIGI packet and message communication, IG lifecycle, and view/HUD integration.
 * Without this game mode, all CIGI packets and messages are disabled.
 */
UCLASS(minimalapi)

class AunrealcigiGameMode : public AGameModeBase
{
  GENERATED_BODY()

public:
  /**
   * @brief Constructor.
   */
  AunrealcigiGameMode();

  /**
   * @brief Starts up the CIGI image generator when entering play mode.
   */
  virtual void BeginPlay() override;

  /**
   * @brief Shuts down the CIGI image generator when exiting play mode.
   * @param EndPlayReason Reason for ending play.
   */
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  /**
   * @brief Updates the CIGI image generator every tick.
   * Handles all communications with the CIGI Host.
   * @param DeltaSeconds Time since last tick.
   */
  virtual void Tick(float DeltaSeconds) override;

private:
  /**
   * @brief Tracks whether the event handler has been initialized (must occur after BeginPlay).
   */
  bool eventHandlerInitialized = false;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026