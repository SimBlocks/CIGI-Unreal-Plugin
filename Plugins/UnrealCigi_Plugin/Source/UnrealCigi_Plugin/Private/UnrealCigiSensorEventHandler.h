//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "EngineLib/ImageGeneratorMessages.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;

    /**
     * @class CUnrealCigiSensorEventHandler
     * @brief Handles CIGI sensor messages for the active Unreal image generator.
     */
    class CUnrealCigiSensorEventHandler
    {
    public:
      /**
       * @brief Construct a new CUnrealCigiSensorEventHandler object.
       * @param eventHandler Reference to the CUnrealCigiEventHandler instance.
       */
      explicit CUnrealCigiSensorEventHandler(CUnrealCigiEventHandler& eventHandler);

      /**
       * @brief Handles the update sensor message.
       * @param data The sensor message data.
       */
      void OnUpdateSensorMessage(const sbio::ig::sensor::SUpdateSensorMessage& data);

      /**
       * @brief Handles the update sensor component message.
       * @param data The sensor component message data.
       */
      void OnUpdateSensorComponentMessage(const sbio::ig::sensor::SUpdateSensorComponentMessage& data);

      /**
       * @brief Handles the creation of a motion tracker view message.
       * @param data The motion tracker view message data.
       */
      void OnCreateMotionTrackerViewMessage(const sbio::ig::sensor::SCreateMotionTrackerViewMessage& data);

      /**
       * @brief Handles the creation of a motion tracker view group message.
       * @param data The motion tracker view group message data.
       */
      void OnCreateMotionTrackerViewGroupMessage(const sbio::ig::sensor::SCreateMotionTrackerViewGroupMessage& data);

      /**
       * @brief Handles the setting of a motion tracker message.
       * @param data The motion tracker message data.
       */
      void OnSetMotionTrackerMessage(const sbio::ig::sensor::SSetMotionTrackerMessage& data);

    private:
      CUnrealCigiEventHandler& EventHandler;  ///< Reference to the CUnrealCigiEventHandler instance.
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026