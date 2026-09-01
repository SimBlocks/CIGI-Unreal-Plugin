//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "EngineLib/ImageGeneratorMessages.h"

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CUnrealCigiEnvironmentEventHandler
     * @brief Handles CIGI environment messages and updates Unreal environment state.
     */
    class CUnrealCigiEnvironmentEventHandler
    {
    public:
      /**
       * @brief Handles the reception of a SetAtmosphereEnabled message.
       * @param data The message data containing the new atmosphere enabled state.
       */
      void OnSetAtmosphereEnabledMessage(const sbio::ig::atmosphere::SSetAtmosphereEnabledMessage& data);

      /**
       * @brief Handles the reception of a SetAtmosphere message.
       * @param data The message data containing the new atmosphere parameters.
       */
      void OnSetAtmosphereMessage(const sbio::ig::atmosphere::SSetAtmosphereMessage& data);

      /**
       * @brief Handles the reception of a SetWeather message.
       * @param data The message data containing the new weather parameters.
       */
      void OnSetWeatherMessage(const sbio::ig::atmosphere::SSetWeatherMessage& data);

      /**
       * @brief Handles the reception of a SetRegionalLayeredWeatherComponentState message.
       * @param data The message data containing the new state for the regional layered weather component.
       */
      void OnSetRegionalLayeredWeatherComponentStateMessage(const sbio::ig::atmosphere::SSetRegionalLayeredWeatherComponentStateMessage& data);

      /**
       * @brief Handles the reception of a SetGlobalLayeredWeatherComponentState message.
       * @param data The message data containing the new state for the global layered weather component.
       */
      void OnSetGlobalLayeredWeatherComponentStateMessage(const sbio::ig::atmosphere::SSetGlobalLayeredWeatherComponentStateMessage& data);

      /**
       * @brief Handles the reception of a SetAtmosphereComponentState message.
       * @param data The message data containing the new state for the atmosphere component.
       */
      void OnSetAtmosphereComponentStateMessage(const sbio::ig::atmosphere::SSetAtmosphereComponentStateMessage& data);

      /**
       * @brief Handles the reception of a SetMaritimeSurfaceConditions message.
       * @param data The message data containing the new maritime surface conditions.
       */
      void OnSetMaritimeSurfaceConditionsMessage(const sbio::ig::ocean::SSetMaritimeSurfaceConditionsMessage& data);

      /**
       * @brief Handles the reception of a SetRegionMaritimeComponentState message.
       * @param data The message data containing the new state for the regional maritime component.
       */
      void OnSetRegionMaritimeComponentStateMessage(const sbio::ig::ocean::SSetRegionMaritimeComponentStateMessage& data);

      /**
       * @brief Handles the reception of a SetGlobalMaritimeComponentState message.
       * @param data The message data containing the new state for the global maritime component.
       */
      void OnSetGlobalMaritimeComponentStateMessage(const sbio::ig::ocean::SSetGlobalMaritimeComponentStateMessage& data);

      /**
       * @brief Handles the reception of a SetEarthReferenceModel message.
       * @param data The message data containing the new earth reference model parameters.
       */
      void OnSetEarthReferenceModelMessage(const sbio::ig::earth::SSetEarthReferenceModelMessage& data);
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026