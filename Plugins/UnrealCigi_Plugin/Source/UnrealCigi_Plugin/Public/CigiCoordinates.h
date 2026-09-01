//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"
#include "MathLib/MathTypes.h"
#include "UnrealCoordinates.h"

class AGeoReferencingSystem;

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CigiCoordinates
     * @brief Provides conversions between CIGI coordinate systems and Unreal world coordinates.
     *
     * Uses the discovered GeoReferencingSystem when available and otherwise uses the configured
     * local tangent plane reference.
     */
    class UNREALCIGI_PLUGIN_API CigiCoordinates
    {
    public:
      /** Searches the world for a GeoReferencingSystem actor and stores the result for conversions. */
      static void TryFindGeoReferencingSystem();

      /**
       * @brief Returns the stored GeoReferencingSystem.
       * @return The stored system, or nullptr when none was found.
       */
      static AGeoReferencingSystem* GetGeoReferencingSystem();

      /**
       * @brief Sets the geodetic origin used by local tangent plane conversions.
       * @param originJson The geodetic origin to use.
       */
      static void SetGeodeticOrigin(sbio::math::SGeodeticCoordinates originJson);

      /**
       * @brief Returns the configured geodetic origin.
       * @return The current geodetic origin.
       */
      static sbio::math::SGeodeticCoordinates GetGeodeticOrigin();

      /** Converts from CIGI Geodetic to the current Unreal Engine coordinate system.
       * If a GeoReferencingSystem exists, it determines the coordinate system.
       * Otherwise, UnrealCigi defaults to a Local Tangent Plane (LTP) system.
       */
      static FUEWorldCoordinates GeodeticToEngine(sbio::math::SGeodeticCoordinates geodetic);

      /** Converts from CIGI Geocentric to the current Unreal Engine coordinate system.
       * If a GeoReferencingSystem exists, it determines the coordinate system.
       * Otherwise, UnrealCigi defaults to a Local Tangent Plane (LTP) system.
       */
      static FUEWorldCoordinates GeocentricToEngine(sbio::math::GeocentricCoordinates geocentric);

      /** Converts from the current Unreal Engine coordinate system to CIGI Geodetic.*/
      static sbio::math::SGeodeticCoordinates EngineToGeodetic(const FUEWorldCoordinates& engine);

      /** Converts from the current Unreal Engine coordinate system to CIGI Geocentric.*/
      static sbio::math::GeocentricCoordinates EngineToGeocentric(const FUEWorldCoordinates& engine);

      /** Converts CIGI reference-plane coordinates (north/east/down) to Unreal engine coordinates. */
      static FUEWorldCoordinates ReferencePlaneToEngine(const sbio::math::ReferencePlaneCoordinates& referencePlane);

      /** Converts Unreal engine coordinates to CIGI reference-plane coordinates (north/east/down). */
      static sbio::math::ReferencePlaneCoordinates EngineToReferencePlane(const FUEWorldCoordinates& engine);

      /**
       * @brief Converts geocentric coordinates to local tangent plane coordinates.
       * @param geocentric Geocentric coordinates struct.
       * @return Local tangent plane coordinates as strongly typed Unreal world coordinates.
       */
      static FUEWorldCoordinates GeocentricToLocalTangentPlane(sbio::math::GeocentricCoordinates geocentric);

      /**
       * @brief Converts geodetic coordinates to local tangent plane coordinates.
       * @param point Geodetic coordinates struct.
       * @return Local tangent plane coordinates as strongly typed Unreal world coordinates.
       */
      static FUEWorldCoordinates GeodeticToLocalTangentPlane(sbio::math::SGeodeticCoordinates point);

      /**
       * @brief Converts local tangent plane coordinates to geocentric coordinates.
       * @param local Local tangent plane coordinates as strongly typed Unreal world coordinates.
       * @return Geocentric coordinates struct.
       */
      static sbio::math::GeocentricCoordinates LocalTangentPlaneToGeocentric(const FUEWorldCoordinates& local);

      /**
       * @brief Converts local tangent plane coordinates to geodetic coordinates.
       * @param local Local tangent plane coordinates as strongly typed Unreal world coordinates.
       * @return Geodetic coordinates struct.
       */
      static sbio::math::SGeodeticCoordinates LocalTangentPlaneToGeodetic(const FUEWorldCoordinates& local);

    private:
      /** The static instance of this class used to store data like the JSON Origin. */
      static CigiCoordinates instance;

      /** Once we find a GRS, we save it here to be used for coordinate conversions. */
      TWeakObjectPtr<AGeoReferencingSystem> savedGRS;

      /** The geodetic origin of the current database.
       * If a GeoReferencingSystem is detected, that origin will be used.
       * Otherwise, geodetic origin will be read from UnrealCigi.config.json.
       */
      sbio::math::SGeodeticCoordinates GeodeticOrigin = sbio::math::SGeodeticCoordinates();

      /** Reference plane used for local tangent plane conversions when no GeoReferencingSystem exists. */
      sbio::math::SReferencePlaneCoordinateSystem ReferencePlane = sbio::math::SReferencePlaneCoordinateSystem();
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026