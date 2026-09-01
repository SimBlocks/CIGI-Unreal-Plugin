//Copyright SimBlocks LLC 2016-2026

#include "CigiBPLib.h"
#include "UnrealCigiEnvironmentManager.h"
#include "UnrealCigiEntityManager.h"
#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigi_Plugin/Public/CigiEntity.h"
#include "CigiController.h"
#include "CigiCoordinates.h"
#include "CigiEntity.h"

using namespace sbio;
using namespace sbio::unrealcigi;

ACigiEntity* UCigiBPLib::FindEntityFromID(int64 entityID)
{
  // Check if the event handler is available
  if (FUnrealCigi_PluginModule::globals.pUnrealEntityManager == nullptr)
  {
    return nullptr;
  }

  // Find the entity using the UnrealCigi entity manager
  return FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(EntityID(entityID));
}

FWeatherLayer UCigiBPLib::WeatherLayerFromID(int layerID)
{
  // Check if the event handler is available
  CUnrealCigiEventHandler* pEventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (pEventHandler == nullptr)
  {
    return FWeatherLayer();
  }

  // Check if the environment manager is available
  if (FUnrealCigi_PluginModule::globals.pEnvironmentManager == nullptr)
  {
    return FWeatherLayer();
  }

  // Retrieve the weather layer from the environment manager
  return FUnrealCigi_PluginModule::globals.pEnvironmentManager->GetWeatherLayer(layerID);
}

TMap<int, FWeatherLayer> UCigiBPLib::GetAllWeatherLayers()
{
  // Check if the event handler is available
  CUnrealCigiEventHandler* pEventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (pEventHandler == nullptr)
  {
    return TMap<int, FWeatherLayer>();
  }

  // Check if the environment manager is available
  if (FUnrealCigi_PluginModule::globals.pEnvironmentManager == nullptr)
  {
    return TMap<int, FWeatherLayer>();
  }

  return FUnrealCigi_PluginModule::globals.pEnvironmentManager->GetWeatherLayers();
}

FWeatherLayer UCigiBPLib::GetAtmosphereInfo()
{
  // Check if the event handler is available
  CUnrealCigiEventHandler* pEventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (pEventHandler == nullptr)
  {
    return FWeatherLayer();
  }

  // Check if the environment manager is available
  const CUnrealCigiEnvironmentManager* environmentManager = FUnrealCigi_PluginModule::globals.pEnvironmentManager.get();
  if (environmentManager == nullptr)
  {
    return FWeatherLayer();
  }
  return environmentManager->AtmosphereInfo;
}

bool UCigiBPLib::IsAtmosphereEnabled()
{
  // Check if the event handler is available
  CUnrealCigiEventHandler* pEventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (pEventHandler == nullptr)
  {
    return false;
  }

  // Check if the environment manager is available and if the atmosphere is enabled
  const CUnrealCigiEnvironmentManager* environmentManager = FUnrealCigi_PluginModule::globals.pEnvironmentManager.get();
  return environmentManager != nullptr && environmentManager->AtmosphereEnabled;
}

float UCigiBPLib::GetWindSpeed()
{
  float highestWindSpeed = 0;

  // If the atmosphere is enabled
  if (IsAtmosphereEnabled())
  { 
    FWeatherLayer atmosphereInfo = GetAtmosphereInfo();

    // Check if the atmosphere's wind speed is higher than the current highest wind speed
    if (atmosphereInfo.HorizontalWindSpeed > highestWindSpeed)
    {
      highestWindSpeed = atmosphereInfo.HorizontalWindSpeed;
    }
  }

  // If the atmosphere is not enabled, return the wind speed of the first enabled weather layer
  TMap<int, FWeatherLayer> layers = GetAllWeatherLayers();
  for (auto& layer : layers)
  {
    // Skip this layer if it is not enabled
    if (!layer.Value.WeatherEnabled)
    {
      continue;
    }

    // Check if this layer's wind speed is higher than the current highest wind speed
    if (layer.Value.HorizontalWindSpeed > highestWindSpeed)
    {
      highestWindSpeed = layer.Value.HorizontalWindSpeed;
    }
  }

  return highestWindSpeed;
}

float UCigiBPLib::GetWindYaw()
{
  // If the atmosphere is enabled, return the wind direction of the atmosphere
  if (IsAtmosphereEnabled())
  {
    FWeatherLayer atmosphereInfo = GetAtmosphereInfo();
    // CIGI windDirection is degrees clockwise from north (looking top-down)
    // In Unreal, X+ is north and Yaw (aka World Rotation Z-axis) is degrees clockwise from X+ (looking top-down)
    // So it's a 1-to-1 conversion from CIGI to Unreal
    return atmosphereInfo.WindDirection;
  }

  // If the atmosphere is not enabled, return the wind direction of the first enabled weather layer
  TMap<int, FWeatherLayer> layers = GetAllWeatherLayers();
  for (auto& layer : layers)
  {
    if (!layer.Value.WeatherEnabled)
    {
      continue;
    }

    return layer.Value.WindDirection;
  }

  // No direction was found
  return 0;
}

FString UCigiBPLib::Conv_SisoEntityEnumerationToString(FSisoID sisoID)
{
  return sisoID.ToString();
}

FString UCigiBPLib::Conv_ComponentDataToString(FComponentData componentData)
{
  return componentData.ToString();
}

FString UCigiBPLib::Conv_ComponentMessageToString(FComponentMessage componentMessage)
{
  return componentMessage.ToString();
}

FString UCigiBPLib::Conv_ComponentClassToString(ComponentClass componentClass)
{
  // Convert the ComponentClass enum to a human-readable string
  switch (componentClass)
  {
  case ComponentClass::ENTITY:
    return "Entity";
  case ComponentClass::VIEW:
    return "View";
  case ComponentClass::VIEW_GROUP:
    return "View Group";
  case ComponentClass::SENSOR:
    return "Sensor";
  case ComponentClass::REGIONAL_MARITIME:
    return "Regional Maritime";
  case ComponentClass::REGIONAL_TERRAIN:
    return "Regional Terrain";
  case ComponentClass::REGIONAL_WEATHER:
    return "Regional Weather";
  case ComponentClass::GLOBAL_MARITIME:
    return "Global Maritime";
  case ComponentClass::GLOBAL_TERRAIN:
    return "Global Terrain";
  case ComponentClass::GLOBAL_WEATHER:
    return "Global Weather";
  case ComponentClass::ATMOSPHERE:
    return "Atmosphere";
  case ComponentClass::CELESTIAL_SPHERE:
    return "Celestial Sphere";
  case ComponentClass::EVENT:
    return "Event";
  case ComponentClass::SYSTEM:
    return "System";
  case ComponentClass::SYMBOL_SURFACE:
    return "Symbol Surface";
  case ComponentClass::SYMBOL:
    return "Symbol";
  }
  return "Unknown Component Class";
}

void UCigiBPLib::GeodeticToEngine(FVector LonLatAlt, /*out*/ FVector& XYZ)
{
  // Convert from geodetic coordinates (Lon, Lat, Alt) to Unreal Engine coordinates
  sbio::math::SGeodeticCoordinates geodetic(Latitude(LonLatAlt.Y), Longitude(LonLatAlt.X), LonLatAlt.Z);
  XYZ = CigiCoordinates::GeodeticToEngine(geodetic).ToFVector();
}

void UCigiBPLib::EngineToGeodetic(FVector XYZ, /*out*/ FVector& LonLatAlt)
{
  // Convert from Unreal Engine coordinates to geodetic coordinates
  sbio::math::SGeodeticCoordinates geodetic = CigiCoordinates::EngineToGeodetic(FUEWorldCoordinates::From(XYZ));
  LonLatAlt = FVector(geodetic.longitude.Value(), geodetic.latitude.Value(), geodetic.altitude.Value());
}

FVector UCigiBPLib::GetCigiOrigin()
{
  // Get the current geodetic origin from CigiCoordinates and return it as a FVector (Lon, Lat, Alt)
  sbio::math::SGeodeticCoordinates origin = CigiCoordinates::GetGeodeticOrigin();
  return FVector(origin.longitude.Value(), origin.latitude.Value(), origin.altitude.Value());
}

void UCigiBPLib::SetLocalTangentPlaneOrigin(FVector LonLatAlt)
{
  // Convert from LonLatAlt to geodetic coordinates
  sbio::math::SGeodeticCoordinates newOrigin;
  newOrigin.longitude = Longitude(LonLatAlt.X);
  newOrigin.latitude = Latitude(LonLatAlt.Y);
  newOrigin.altitude = HeightRelativeToWGS84Ellipsoid(LonLatAlt.Z);
  CigiCoordinates::SetGeodeticOrigin(newOrigin);
}

void UCigiBPLib::GeodeticToLocalTangentPlane(FVector LonLatAlt, /*out*/ FVector& XYZ)
{
  // Convert from LonLatAlt to geodetic coordinates
  sbio::math::SGeodeticCoordinates geodeticPoint;
  geodeticPoint.longitude = Longitude(LonLatAlt.X);
  geodeticPoint.latitude = Latitude(LonLatAlt.Y);
  geodeticPoint.altitude = HeightRelativeToWGS84Ellipsoid(LonLatAlt.Z);
  XYZ = CigiCoordinates::GeodeticToLocalTangentPlane(geodeticPoint).ToFVector();
}

void UCigiBPLib::LocalTangentPlaneToGeodetic(FVector XYZ, /*out*/ FVector& LonLatAlt)
{
  // Convert from local tangent plane coordinates to geodetic coordinates
  sbio::math::SGeodeticCoordinates geodeticPoint = CigiCoordinates::LocalTangentPlaneToGeodetic(FUEWorldCoordinates::From(XYZ));
  LonLatAlt = FVector(geodeticPoint.longitude.Value(), geodeticPoint.latitude.Value(), geodeticPoint.altitude.Value());
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026