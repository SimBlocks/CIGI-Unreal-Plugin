//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "SymbolLib/SymbolTypes.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;

    /**
     * @class CUnrealCigiSymbolSurfacePresenter
     * @brief Presents a CIGI symbol surface through its associated Unreal widget.
     */
    class CUnrealCigiSymbolSurfacePresenter
    {
    public:
      /**
       * @brief Creates a presenter associated with an UnrealCigi event handler.
       * @param eventHandler Event handler used to access the current Unreal world.
       */
      explicit CUnrealCigiSymbolSurfacePresenter(CUnrealCigiEventHandler& eventHandler);

      /**
       * @brief Creates or updates the Unreal widget associated with a symbol surface.
       *
       * Creates a widget when necessary, reconfigures it when the surface type changes,
       * attaches view widgets to the appropriate player controller, and updates entity
       * widget components for billboard and world surfaces. The function returns without
       * changes when the symbol manager, surface, or required widget is unavailable.
       *
       * @param surfaceID Identifier of the symbol surface to present.
       * @param newAttachID Entity or view identifier to which the surface is attached.
       */
      void UpdateSurfaceWidget(sbio::symbol::SymbolSurfaceID surfaceID, int32 newAttachID);

      /**
       * @brief Removes the Unreal widget associated with a symbol surface.
       *
       * Removes the widget from its parent and detaches entity widget components for
       * billboard and world surfaces. The function returns without changes when the
       * symbol manager, surface, or widget is unavailable.
       *
       * @param surfaceID Identifier of the symbol surface whose widget should be removed.
       */
      void RemoveSurfaceWidget(sbio::symbol::SymbolSurfaceID surfaceID);

    private:
      CUnrealCigiEventHandler& EventHandler;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026