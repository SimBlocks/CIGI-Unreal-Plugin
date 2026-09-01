//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "EngineLib/ImageGeneratorMessages.h"
#include "CigiSymbol.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;

    /**
     * @class CUnrealCigiSymbolEventHandler
     * @brief Handles CIGI symbol messages and updates symbol rendering state.
     */
    class CUnrealCigiSymbolEventHandler
    {
    public:
      explicit CUnrealCigiSymbolEventHandler(CUnrealCigiEventHandler& eventHandler);

      /**
       * @brief Handles the creation of a symbol text message.
       * @param data The data of the message received.
       */
      void OnCreateSymbolTextMessage(const sbio::ig::symbol::SCreateSymbolTextMessage& data);

      /**
       * @brief Handles the update of a symbol text message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolTextMessage(const sbio::ig::symbol::SUpdateSymbolTextMessage& data);

      /**
       * @brief Handles the creation of a symbol circle message.
       * @param data The data of the message received.
       */
      void OnCreateSymbolCircleMessage(const sbio::ig::symbol::SCreateSymbolCircleMessage& data);

      /**
       * @brief Handles the update of a symbol circle message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolCircleMessage(const sbio::ig::symbol::SUpdateSymbolCircleMessage& data);

      /**
       * @brief Handles the update of a symbol circle element message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolCircleElementMessage(const sbio::ig::symbol::SUpdateSymbolCircleElementMessage& data);

      /**
       * @brief Handles the update of a symbol circle filled message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolCircleFilledMessage(const sbio::ig::symbol::SUpdateSymbolCircleFilledMessage& data);

      /**
       * @brief Handles the update of a symbol circle filled element message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolCircleFilledElementMessage(const sbio::ig::symbol::SUpdateSymbolCircleFilledElementMessage& data);

      /**
       * @brief Handles the creation of a symbol textured circle message.
       * @param data The data of the message received.
       */
      void OnCreateSymbolTexturedCircleMessage(const sbio::ig::symbol::SCreateSymbolTexturedCircleMessage& data);

      /**
       * @brief Handles the update of a symbol textured circle message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolTexturedCircleMessage(const sbio::ig::symbol::SUpdateSymbolTexturedCircleMessage& data);

      /**
       * @brief Handles the update of a textured circle message.
       * @param data The data of the message received.
       */
      void OnUpdateTexturedCircleMessage(const sbio::ig::symbol::SUpdateTexturedCircleMessage& data);

      /**
       * @brief Handles the creation of a symbol polygon message.
       * @param data The data of the message received.
       */
      void OnCreateSymbolPolygonMessage(const sbio::ig::symbol::SCreateSymbolPolygonMessage& data);

      /**
       * @brief Handles the update of a symbol polygon message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolPolygonMessage(const sbio::ig::symbol::SUpdateSymbolPolygonMessage& data);

      /**
       * @brief Handles the setting of a symbol polygon vertex message.
       * @param data The data of the message received.
       */
      void OnSetSymbolPolygonVertexMessage(const sbio::ig::symbol::SSetSymbolPolygonVertexMessage& data);

      /**
       * @brief Handles the creation of a symbol textured polygon message.
       * @param data The data of the message received.
       */
      void OnCreateSymbolTexturedPolygonMessage(const sbio::ig::symbol::SCreateSymbolTexturedPolygonMessage& data);

      /**
       * @brief Handles the update of a symbol textured polygon message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolTexturedPolygonMessage(const sbio::ig::symbol::SUpdateSymbolTexturedPolygonMessage& data);

      /**
       * @brief Handles the setting of a symbol textured polygon vertex message.
       * @param data The data of the message received.
       */
      void OnSetSymbolTexturedPolygonVertexMessage(const sbio::ig::symbol::SSetSymbolTexturedPolygonVertexMessage& data);

      /**
       * @brief Handles the update of an entity billboard symbol surface message.
       * @param data The data of the message received.
       */
      void OnUpdateEntityBillboardSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateEntityBillboardSymbolSurfaceMessage& data);

      /**
       * @brief Handles the creation of a symbol surface message.
       * @param data The data of the message received.
       */
      void OnCreateSymbolSurfaceMessage(const sbio::ig::symbol::SCreateSymbolSurfaceMessage& data);

      /**
       * @brief Handles the destruction of a symbol surface message.
       * @param data The data of the message received.
       */
      void OnDestroySymbolSurfaceMessage(const sbio::ig::symbol::SDestroySymbolSurfaceMessage& data);

      /**
       * @brief Handles the update of a symbol surface message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateSymbolSurfaceMessage& data);

      /**
       * @brief Handles the update of a view symbol surface message.
       * @param data The data of the message received.
       */
      void OnUpdateViewSymbolSurfaceMessage(const sbio::ig::symbol::SUpdateViewSymbolSurfaceMessage& data);

      /**
       * @brief Handles the setting of a symbol color message.
       * @param data The data of the message received.
       */
      void OnSetSymbolColorMessage(const sbio::ig::symbol::SSetSymbolColorMessage& data);

      /**
       * @brief Handles the destruction of a symbol message.
       * @param data The data of the message received.
       */
      void OnDestroySymbolMessage(const sbio::ig::symbol::SDestroySymbolMessage& data);

      /**
       * @brief Handles the setting of a symbol visible message.
       * @param data The data of the message received.
       */
      void OnSetSymbolVisibleMessage(const sbio::ig::symbol::SSetSymbolVisibleMessage& data);

      /**
       * @brief Handles the setting of a symbol attached message.
       * @param data The data of the message received.
       */
      void OnSetSymbolAttachedMessage(const sbio::ig::symbol::SSetSymbolAttachedMessage& data);

      /**
       * @brief Handles the setting of a symbol unattached message.
       * @param data The data of the message received.
       */
      void OnSetSymbolUnattachedMessage(const sbio::ig::symbol::SSetSymbolUnattachedMessage& data);

      /**
       * @brief Handles the setting of a symbol surface message.
       * @param data The data of the message received.
       */
      void OnSetSymbolSurfaceMessage(const sbio::ig::symbol::SSetSymbolSurfaceMessage& data);

      /**
       * @brief Handles the setting of a top-level symbol transform message.
       * @param data The data of the message received.
       */
      void OnSetTopLevelSymbolTransformMessage(const sbio::ig::symbol::SSetTopLevelSymbolTransformMessage& data);

      /**
       * @brief Handles the setting of a child symbol transform message.
       * @param data The data of the message received.
       */
      void OnSetChildSymbolTransformMessage(const sbio::ig::symbol::SSetChildSymbolTransformMessage& data);

      /**
       * @brief Handles the update of a symbol message.
       * @param data The data of the message received.
       */
      void OnUpdateSymbolMessage(const sbio::ig::symbol::SUpdateSymbolMessage& data);

      /**
       * @brief Handles the setting of a symbol component state message.
       * @param data The data of the message received.
       */
      void OnSetSymbolComponentStateMessage(const sbio::ig::symbol::SSetSymbolComponentStateMessage& data);

      /**
       * @brief Handles the setting of a symbol surface component state message.
       * @param data The data of the message received.
       */
      void OnSetSymbolSurfaceComponentStateMessage(const sbio::ig::symbol::SSetSymbolSurfaceComponentStateMessage& data);

      /**
       * @brief Handles the creation of a symbol from template message.
       * @param data The data of the message received.
       */
      void OnCreateSymbolFromTemplateMessage(const sbio::ig::symbol::SCreateSymbolFromTemplateMessage& data);

      /**
       * @brief Updates the symbol surface widget.
       * @param surfaceID The surface ID of the symbol.
       * @param newAttachID The new attach ID to be set.
       */
      void UpdateSymbolSurfaceWidget(sbio::symbol::SymbolSurfaceID surfaceID, int32 newAttachID);

      /**
       * @brief Removes the symbol surface widget.
       * @param surfaceID The surface ID of the symbol to be removed.
       */
      void RemoveSymbolSurfaceWidget(sbio::symbol::SymbolSurfaceID surfaceID);

    private:
      sbio::symbol::CSymbol* FindSymbol(sbio::symbol::SymbolID symbolID) const;

      CUnrealCigiEventHandler& EventHandler;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026