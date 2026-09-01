//Copyright SimBlocks LLC 2016-2026
/**
 * @file unrealcigiPlayerController.h
 * @brief Defines the AunrealcigiPlayerController class for SimBlocks UnrealCIGI plugin player controller integration.
 *
 * This header provides:
 * - The AunrealcigiPlayerController class, derived from APlayerController, for managing CIGI ViewID assignment and symbol surface widgets per player.
 * - Functions for assigning and retrieving CIGI ViewIDs, and for managing symbol surface widgets.
 *
 * Usage:
 * - Use AunrealcigiPlayerController to associate a player with a CIGI ViewID and manage their symbol surface widgets.
 * - SetViewID assigns a CIGI ViewID to the player controller.
 * - AddWidget and RemoveAllWidgets manage symbol surface widgets for the player's view.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CigiWidget.h"
#include "unrealcigiPlayerController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCigiPlayerController, Log, All)

/**
 * @class AunrealcigiPlayerController
 * @brief Player controller class for CIGI view and symbol surface management.
 *
 * Associates a player with a CIGI ViewID and manages symbol surface widgets for that view.
 */
UCLASS()

class AunrealcigiPlayerController : public APlayerController
{
  GENERATED_BODY()

public:
  /**
   * @brief Constructor.
   */
  AunrealcigiPlayerController();

  /**
   * @brief Assigns a CIGI ViewID to this player.
   * Each ViewID corresponds to one player, one pawn, and one camera component.
   * @param viewID The CIGI ViewID to assign.
   */
  void SetViewID(sbio::ViewID viewID);

  /**
   * @brief Gets the CIGI ViewID assigned to this player controller.
   * @return The assigned CIGI ViewID.
   */
  sbio::ViewID GetViewID() const;

  /**
   * @brief Assigns a CIGI Symbol Surface (symbolSurfaceID) to this player controller's ViewID.
   * @param symbolSurfaceID The symbol surface ID to add.
   */
  void AddWidget(int symbolSurfaceID);

  /**
   * @brief Removes all CIGI Symbol Surfaces from this player controller.
   */
  void RemoveAllWidgets();

private:
  /**
   * @brief The ViewID assigned to this player controller.
   */
  sbio::ViewID ViewID = sbio::UnknownViewID;

  /**
   * @brief Map of SymbolSurfaceIDs to their associated widgets for this player controller.
   */
  TMap<int32, UCigiWidget*> Widgets;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026