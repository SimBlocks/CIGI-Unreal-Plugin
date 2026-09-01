//Copyright SimBlocks LLC 2016-2026
/**
 * @file CigiHUD.h
 * @brief Defines the ACigiHUD class for custom HUD rendering in the SimBlocks CIGI Unreal plugin.
 *
 * This header provides:
 * - The ACigiHUD class, derived from Unreal's AHUD, for drawing custom HUD elements associated with a CIGI View.
 * - Utility functions for viewport size and coordinate conversion.
 * - Integration with CIGI event handling and view management.
 *
 * Usage:
 * - ACigiHUD is used to render HUD overlays for a specific CIGI ViewID.
 * - Override DrawHUD to implement custom drawing logic.
 * - Use GetActiveViewportSize and PerToPix for screen-space calculations.
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ModuleAPI.h"
#include "UnrealCigi_Declarations.h"
#include "ViewLib\ViewTypes.h"
#include "CigiHUD.generated.h"

/**
 * @class ACigiHUD
 * @brief HUD class for rendering overlays and information for a CIGI View.
 *
 * ACigiHUD provides custom HUD drawing and utility functions for screen-space calculations.
 * The ViewID property links the HUD to a specific CIGI view.
 */
UCLASS()

class MODULE_API ACigiHUD : public AHUD
{
  GENERATED_BODY()

public:
  /**
   * @brief The CIGI ViewID this HUD is attached to.
   * Assigned in AunrealcigiPlayerController::SetViewID.
   */
  sbio::ViewID ViewID = sbio::UnknownViewID;

  /**
   * @brief Default constructor.
   */
  ACigiHUD();

protected:
  /**
   * @brief Draws the HUD overlay.
   * Override to implement custom HUD rendering.
   */
  virtual void DrawHUD() override;

  /**
   * @brief Returns the size of the active viewport, excluding black bars.
   * @return Viewport size in pixels as FVector2D.
   */
  FVector2D GetActiveViewportSize();

  /**
   * @brief Converts screen coordinates from percentages to pixels.
   * @param percentX X percentage (0.0 to 1.0).
   * @param percentY Y percentage (0.0 to 1.0).
   * @return Pixel coordinates as FVector2D.
   */
  FVector2D PerToPix(float percentX, float percentY);
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026