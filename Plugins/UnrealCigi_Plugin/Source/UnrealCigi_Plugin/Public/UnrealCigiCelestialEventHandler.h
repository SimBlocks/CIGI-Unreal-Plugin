//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "ModuleAPI.h"
#include "MathLib/MathTypes.h"
#include "EngineLib/ImageGeneratorMessages.h"
#include "EngineLib/IImageGeneratorEventMessenger.h"

class UWorld;

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CUnrealCigiCelestialEventHandler
     * @brief Handles CIGI celestial-sphere messages for the Unreal scene.
     */
    class CUnrealCigiCelestialEventHandler
    {
    public:
      /**
       * @brief Applies a celestial-sphere update to the Unreal world.
       * @param data Celestial-sphere message data.
       * @param WorldRef Unreal world to update.
       */
      void UpdateCelestialSphereMessage(const sbio::ig::celestial::SUpdateCelestialSphereMessage& data, UWorld* WorldRef);

      /**
       * @brief Applies a date-and-time update to the Unreal world.
       * @param data Date-and-time message data.
       * @param WorldRef Unreal world to update.
       */
      void UpdateDateTimeMessage(const sbio::ig::celestial::SUpdateDateTimeMessage& data, UWorld* WorldRef);

      /**
       * @brief Applies latitude and longitude values to the celestial environment.
       * @param latitude Latitude to apply.
       * @param Longitude Longitude to apply.
       * @param WorldRef Unreal world to update.
       */
      void UpdateLatLon(sbio::math::Latitude latitude, sbio::math::Longitude Longitude, UWorld* WorldRef);
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026