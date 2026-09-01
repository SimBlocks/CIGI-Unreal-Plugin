//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "EngineLib/ImageGeneratorMessages.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;

    /**
     * @class CUnrealCigiSystemEventHandler
     * @brief Handles CIGI system messages for the Unreal image generator.
     */
    class CUnrealCigiSystemEventHandler
    {
    public:
      explicit CUnrealCigiSystemEventHandler(CUnrealCigiEventHandler& eventHandler);

      /**
       * @brief Handles the reception of a SetEventComponentState message.
       * @param data The message data containing the event component state information.
       */
      void OnSetEventComponentStateMessage(const sbio::ig::system::SSetEventComponentStateMessage& data);

      /**
       * @brief Handles the reception of a SetSystemComponentState message.
       * @param data The message data containing the system component state information.
       */
      void OnSetSystemComponentStateMessage(const sbio::ig::system::SSetSystemComponentStateMessage& data);

      /**
       * @brief Handles the reception of a HostConnected message.
       * @param data The message data containing the host connection information.
       */
      void OnSetHostConnectedMessage(const sbio::ig::network::SHostConnectedMessage& data);

      /**
       * @brief Handles the reception of a HostDisconnected message.
       * @param data The message data containing the host disconnection information.
       */
      void OnSetHostDisconnectedMessage(const sbio::ig::network::SHostDisconnectedMessage& data);

    private:
      CUnrealCigiEventHandler& EventHandler;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026