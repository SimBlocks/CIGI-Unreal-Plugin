//Copyright SimBlocks LLC 2016-2026
#include "CigiCoordinates.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "GeoReferencing/Public/GeoReferencingSystem.h"
#include "unrealcigiUtil.h"
#include "unrealcigiEventHandler.h"
#include "Kismet/GameplayStatics.h"
#include "MathLib/CoordinateConversions.h"
#include "CigiEntity.h"

using namespace sbio::unrealcigi;
using namespace sbio::math;

CigiCoordinates CigiCoordinates::instance = CigiCoordinates();

// ---------- Setup ----------

void CigiCoordinates::TryFindGeoReferencingSystem()
{
  // Get the event handler from the plugin globals. If event handler is null, then a GRS cannot be found.
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (eventHandler == nullptr)
  {
    UE_LOG(LogTemp, Error, TEXT("CigiCoordinates recieved NULL EventHandler!"));
    instance.savedGRS.Reset();
    return;
  }

  // Get the world context from the event handler. If world context is null, then a GRS cannot be found.
  UWorld* worldContext = eventHandler->GetWorld();
  if (worldContext == nullptr)
  {
    UE_LOG(LogTemp, Error, TEXT("CigiCoordinates recieved NULL WorldRef from EventHandler!"));
    instance.savedGRS.Reset();
    return;
  }

  // Search for a GeoReferencingSystem. Use the first one we find, ignore any others. (there should only be one)
  TArray<AActor*> foundActors;
  UGameplayStatics::GetAllActorsOfClass(worldContext, AGeoReferencingSystem::StaticClass(), foundActors);
  for (AActor* actor : foundActors)
  {
    AGeoReferencingSystem* grs = Cast<AGeoReferencingSystem>(actor);
    // If we found a valid grs, then save that as our new reference and exit the function
    if (grs != nullptr)
    {
      instance.savedGRS = grs;
      return;
    }
  }

  // Record "null" to indicate that there isn't a GRS in this level. Overwrites data from previous (unloaded) levels.
  instance.savedGRS.Reset();
}

AGeoReferencingSystem* CigiCoordinates::GetGeoReferencingSystem()
{
  return instance.savedGRS.Get();
}

void CigiCoordinates::SetGeodeticOrigin(sbio::math::SGeodeticCoordinates originJson)
{
  instance.GeodeticOrigin = originJson;
  instance.ReferencePlane = InitReferencePlaneCoordinates(instance.GeodeticOrigin);
}

SGeodeticCoordinates CigiCoordinates::GetGeodeticOrigin()
{
  // If a GeoReferencingSystem exists, use it for the conversion
  AGeoReferencingSystem* grs = GetGeoReferencingSystem();
  if (grs != nullptr)
  {
    return SGeodeticCoordinates(Latitude(grs->OriginLatitude), Longitude(grs->OriginLongitude), grs->OriginAltitude);
  }
  else
  {
    return instance.GeodeticOrigin;
  }
}

// ---------- Main Conversions ----------

FUEWorldCoordinates CigiCoordinates::GeodeticToEngine(SGeodeticCoordinates geodetic)
{
  // If a GeoReferencingSystem exists, use it for the conversion
  AGeoReferencingSystem* grs = GetGeoReferencingSystem();
  if (grs != nullptr)
  {
    FGeographicCoordinates geographic(geodetic.longitude.Value(), geodetic.latitude.Value(), geodetic.altitude.Value());
    FVector engine;
    // The user should have set "Planet Shape", "Projected CRS", and "Geographic CRS" in their GeoReferencingSystem actor.
    grs->GeographicToEngine(geographic, engine);
    return FUEWorldCoordinates(FVector(engine.Y, engine.X, engine.Z));
  }

  // If we do not have a GeoReferencingSystem, then do the default LTP conversion.
  return GeodeticToLocalTangentPlane(geodetic);
}

FUEWorldCoordinates CigiCoordinates::GeocentricToEngine(GeocentricCoordinates geocentric)
{
  AGeoReferencingSystem* grs = GetGeoReferencingSystem();
  if (grs != nullptr)
  {
    // For CIGI, we want to use our own Geocentric space. So first convert to Geodetic.
    SGeodeticCoordinates geodetic = sbio::math::ConvertGeocentricToGeodeticCoordinates(geocentric);
    // Then convert from Geodetic normally.
    return GeodeticToEngine(geodetic);
  }

  // If there is no GeoReferencingSystem, then we only need a single conversion to LTP.
  return GeocentricToLocalTangentPlane(geocentric);
}

SGeodeticCoordinates CigiCoordinates::EngineToGeodetic(const FUEWorldCoordinates& engine)
{
  // If a GeoReferencingSystem exists, use it for the conversion
  AGeoReferencingSystem* grs = GetGeoReferencingSystem();
  if (grs != nullptr)
  {
    FGeographicCoordinates geographic;
    const FVector cigiEngine(engine.ToFVector().Y, engine.ToFVector().X, engine.ToFVector().Z);
    // The user should have set "Planet Shape", "Projected CRS", and "Geographic CRS" in their GeoReferencingSystem actor.
    grs->EngineToGeographic(cigiEngine, geographic);

    SGeodeticCoordinates geodetic(Latitude(geographic.Latitude), Longitude(geographic.Longitude), geographic.Altitude);
    return geodetic;
  }

  // If we do not have a GeoReferencingSystem, then do the default LTP conversion.
  return LocalTangentPlaneToGeodetic(engine);
}

GeocentricCoordinates CigiCoordinates::EngineToGeocentric(const FUEWorldCoordinates& engine)
{
  AGeoReferencingSystem* grs = GetGeoReferencingSystem();
  if (grs != nullptr)
  {
    // First convert to Geodetic normally
    SGeodeticCoordinates geodetic = EngineToGeodetic(engine);
    // Then we use our own Geocentric space for CIGI
    return sbio::math::ConvertGeodeticToGeocentricCoordinates(geodetic);
  }

  // If there is no GeoReferencingSystem, then we only need a single conversion from LTP.
  return LocalTangentPlaneToGeocentric(engine);
}

FUEWorldCoordinates CigiCoordinates::ReferencePlaneToEngine(const ReferencePlaneCoordinates& referencePlane)
{
  return FUEWorldCoordinates(utils::ReferencePlaneCoordinatesToFVector(referencePlane));
}

ReferencePlaneCoordinates CigiCoordinates::EngineToReferencePlane(const FUEWorldCoordinates& engine)
{
  return utils::FVectorToReferencePlaneCoordinates(engine.ToFVector());
}

FUEWorldCoordinates CigiCoordinates::GeocentricToLocalTangentPlane(GeocentricCoordinates geocentric)
{
  sbio::math::ReferencePlaneCoordinates referencePlaneCoords = sbio::math::ConvertGeocentricToReferencePlaneCoordinates(geocentric, instance.ReferencePlane);
  return ReferencePlaneToEngine(referencePlaneCoords);
}

FUEWorldCoordinates CigiCoordinates::GeodeticToLocalTangentPlane(SGeodeticCoordinates point)
{
  return GeocentricToLocalTangentPlane(sbio::math::ConvertGeodeticToGeocentricCoordinates(point));
}

GeocentricCoordinates CigiCoordinates::LocalTangentPlaneToGeocentric(const FUEWorldCoordinates& local)
{
  sbio::math::ReferencePlaneCoordinates referencePlaneCoordinates = EngineToReferencePlane(local);
  return sbio::math::ConvertReferencePlaneToGeocentricCoordinates(referencePlaneCoordinates, instance.ReferencePlane);
}

SGeodeticCoordinates CigiCoordinates::LocalTangentPlaneToGeodetic(const FUEWorldCoordinates& local)
{
  return sbio::math::ConvertGeocentricToGeodeticCoordinates(LocalTangentPlaneToGeocentric(local));
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026