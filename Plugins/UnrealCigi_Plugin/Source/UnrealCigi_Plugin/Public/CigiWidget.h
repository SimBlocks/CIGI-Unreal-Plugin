//Copyright SimBlocks LLC 2016-2026
/**
 * @file CigiWidget.h
 * @brief Defines the UCigiWidget class for rendering CIGI symbol surfaces in Unreal Engine UI.
 *
 * This header provides:
 * - The UCigiWidget class, derived from UUserWidget, for displaying CIGI symbol surfaces using Slate widgets.
 * - Integration with SSlateCigiWidget for custom symbol rendering.
 * - Functions for setting the symbol surface ID and managing Slate resources.
 *
 * Usage:
 * - UCigiWidget is used to display a symbol surface in the Unreal UI.
 * - Call SetSurfaceID before displaying the widget to associate it with a symbol surface.
 * - The widget manages its Slate resources and passes the surface ID to SSlateCigiWidget.
 */

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlateCigiWidget.h"
#include "ModuleAPI.h"
#include "CigiWidget.generated.h"

/**
 * @class UCigiWidget
 * @brief Widget class for rendering a CIGI symbol surface in Unreal UI.
 *
 * UCigiWidget wraps a SSlateCigiWidget for custom symbol rendering.
 * SetSurfaceID must be called before displaying the widget.
 */
UCLASS()

class MODULE_API UCigiWidget : public UUserWidget
{
  GENERATED_BODY()

public:
  /**
   * @brief Called by Unreal when the widget is destroyed.
   * Used to destroy the SSlateCigiWidget.
   * @param bReleaseChildren Whether to release child widgets.
   */
  virtual void ReleaseSlateResources(bool bReleaseChildren) override;

  /**
   * @brief Sets the symbol surface ID for this widget.
   * Must be called before the widget is displayed.
   * @param symbolSurfaceID The symbol surface ID to associate.
   */
  void SetSurfaceID(sbio::symbol::SymbolSurfaceID symbolSurfaceID);

protected:
  /**
   * @brief Holds the SSlateCigiWidget instance for rendering.
   * Resetting this destroys the SSlateCigiWidget.
   */
  TSharedPtr<SSlateCigiWidget> MyCigiWidget;

  /**
   * @brief Called by Unreal when the widget is displayed.
   * Returns the Slate widget to be rendered.
   * @return Shared reference to the Slate widget.
   */
  virtual TSharedRef<SWidget> RebuildWidget() override;

private:
  /**
   * @brief Stores the symbol surface ID set by SetSurfaceID.
   * Passed to SSlateCigiWidget when RebuildWidget is called.
   */
  sbio::symbol::SymbolSurfaceID m_SymbolSurfaceID = sbio::symbol::UnknownSymbolSurfaceID;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026