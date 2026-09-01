//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"
#include "CigiBPLib.h"
#include "EngineLib/ImageGeneratorMessages.h"

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CUnrealCigiEnvironmentManager
     * @brief Maintains environment state used while processing CIGI messages.
     */
    class CUnrealCigiEnvironmentManager
    {
    public:
      /**
       * @brief Reset the environment manager to its default state.
       */
      void Reset();

      /**
       * @brief Enable or disable the atmosphere.
       * @param enabled True to enable the atmosphere, false to disable.
       */
      void SetAtmosphereEnabled(bool enabled);

      /**
       * @brief Set the atmosphere parameters.
       * @param data The atmosphere data to be set.
       */
      void SetAtmosphere(const sbio::ig::atmosphere::SSetAtmosphereMessage& data);

      /**
       * @brief Set the weather parameters.
       * @param data The weather data to be set.
       */
      void SetWeather(const sbio::ig::atmosphere::SSetWeatherMessage& data);

      /**
       * @brief Get a specific weather layer by its ID.
       * @param layerID The ID of the layer to retrieve.
       * @return The weather layer associated with the given ID.
       */
      FWeatherLayer GetWeatherLayer(int32 layerID) const;

      /**
       * @brief Get all weather layers.
       * @return A map of all weather layers, indexed by their ID.
       */
      TMap<int, FWeatherLayer> GetWeatherLayers() const;

      /**
       * A map of weather layers, indexed by their ID.
       */
      TMap<int, FWeatherLayer> WeatherLayers;

      /**
       * Atmosphere information.
       */
      FWeatherLayer AtmosphereInfo;

      /**
       * Is the atmosphere enabled?
       */
      bool AtmosphereEnabled = false;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026