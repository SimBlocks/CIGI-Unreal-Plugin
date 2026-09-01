//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"
#include "EngineLib/ImageGeneratorMessages.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;

    /**
     * @class CUnrealCigiTerrainEventHandler
     * @brief Handles CIGI terrain queries and terrain component messages.
     */
    class CUnrealCigiTerrainEventHandler
    {
    public:
      /**
       * @brief Constructor for CUnrealCigiTerrainEventHandler
       * @param eventHandler Reference to the CUnrealCigiEventHandler instance
       */
      explicit CUnrealCigiTerrainEventHandler(CUnrealCigiEventHandler& eventHandler);

      /**
       * @brief Handles basic line of sight segment request messages
       * @param data The message data containing the line of sight segment request
       */
      void OnLineOfSightSegmentRequestBasicMessage(const sbio::ig::terrain::SLineOfSightSegmentRequestBasicMessage& data);

      /**
       * @brief Handles extended line of sight segment request messages
       * @param data The message data containing the extended line of sight segment request
       */
      void OnLineOfSightSegmentRequestExtendedMessage(const sbio::ig::terrain::SLineOfSightSegmentRequestExtendedMessage& data);

      /**
       * @brief Handles basic line of sight vector request messages
       * @param data The message data containing the line of sight vector request
       */
      void OnLineOfSightVectorRequestBasicMessage(const sbio::ig::terrain::SLineOfSightVectorRequestBasicMessage& data);

      /**
       * @brief Handles extended line of sight vector request messages
       * @param data The message data containing the extended line of sight vector request
       */
      void OnLineOfSightVectorRequestExtendedMessage(const sbio::ig::terrain::SLineOfSightVectorRequestExtendedMessage& data);

      /**
       * @brief Handles height above terrain request messages
       * @param data The message data containing the height above terrain request
       */
      void OnHeightAboveTerrainRequestMessage(const sbio::ig::terrain::SHeightAboveTerrainRequestMessage& data);

      /**
       * @brief Handles height of terrain request messages
       * @param data The message data containing the height of terrain request
       */
      void OnHeightOfTerrainRequestMessage(const sbio::ig::terrain::SHeightOfTerrainRequestMessage& data);

      /**
       * @brief Handles changes in terrestrial surface conditions
       */
      void OnTerrestrialSurfaceConditionsChangedMessage();

      /**
       * @brief Handles setting the state of regional terrain surface components
       * @param data The message data containing the regional terrain surface component state
       */
      void OnSetRegionalTerrainSurfaceComponentStateMessage(const sbio::ig::terrain::SSetRegionalTerrainSurfaceComponentStateMessage& data);

      /**
       * @brief Handles setting the state of global terrain components
       * @param data The message data containing the global terrain component state
       */
      void OnSetGlobalTerrainComponentStateMessage(const sbio::ig::terrain::SSetGlobalTerrainComponentStateMessage& data);

    protected:
      /**
       * @brief Traces the terrain for a single hit result
       * @param world The world context
       * @param start The start location of the trace
       * @param end The end location of the trace
       * @param result The resulting hit information
       * @return True if the trace hits the terrain, false otherwise
       */
      bool TraceTerrain(UWorld* world, const FVector& start, const FVector& end, FHitResult& result);

      /**
       * @brief Traces the terrain for multiple hit results
       * @param world The world context
       * @param start The start location of the trace
       * @param end The end location of the trace
       * @param results The array to store resulting hit information
       * @return True if the trace hits the terrain, false otherwise
       */
      bool TraceTerrain(UWorld* world, const FVector& start, const FVector& end, TArray<FHitResult>& results);

      /**
       * @brief Filters the line of sight hits based on an alpha threshold
       * @param hits The array of initial hit results
       * @param alphaThreshold The threshold for filtering hits
       * @param results The array to store filtered hit results
       */
      void FilterLineOfSightHits(const TArray<FHitResult>& hits, double alphaThreshold, TArray<FHitResult>& results);

    private:
      CUnrealCigiEventHandler& EventHandler; ///< Reference to the event handler instance
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026