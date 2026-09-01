//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiTerrainEventHandler.h"

#include "UnrealCigiEventHandler.h"
#include "UnrealCigiEntityManager.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "CigiCoordinates.h"
#include "UnrealCoordinates.h"
#include "CigiController.h"
#include "CigiEntity.h"
#include "unrealcigiUtil.h"
#include "IGCigiLib/IGCigiLib.h"
#include "IGCigiLib/IGResponseEventDispatcher.h"
#include "CoreMinimal.h"

using namespace sbio;
using namespace sbio::cigi;
using namespace sbio::unrealcigi;
using namespace sbio::unrealcigi::utils;

namespace
{
  uint8 GetCurrentHostFrameLSN();
}

void CUnrealCigiTerrainEventHandler::OnLineOfSightVectorRequestExtendedMessage(const sbio::ig::terrain::SLineOfSightVectorRequestExtendedMessage& data)
{
  // Convert the start and end geocentric coordinates from the CIGI message to Unreal Engine coordinates using the CigiCoordinates utility class.
  const FVector start = CigiCoordinates::GeocentricToEngine(data.Start).ToFVector();
  const FVector end = CigiCoordinates::GeocentricToEngine(data.End).ToFVector();
  TArray<FHitResult> hits;
  TraceTerrain(EventHandler.GetWorld(), start, end, hits);
  TArray<FHitResult> results;
  FilterLineOfSightHits(hits, data.AlphaThreshold, results);
  const int responseCount = results.Num() <= 0 ? 1 : results.Num();

  // Iterate through the results and send appropriate responses based on the hit information and the response coordinate system specified in the request.
  for (int i = 0; i < responseCount; ++i)
  {
    // Determine if the current hit result is valid and retrieve the corresponding hit information, including the entity and range.
    const bool resValid = results.Num() > i;
    const FHitResult& resHit = resValid ? results[i] : FHitResult();
    ACigiEntity* entity = resValid ? Cast<ACigiEntity>(resHit.GetActor()) : nullptr;
    const FVector normal = resValid ? resHit.Normal.Rotation().Euler() : FVector::ZeroVector;
    const double range = resValid ? FVector::Dist(resHit.TraceStart, resHit.Location) / 100.0 : 0.0;
    const uint8 frameLSN = GetCurrentHostFrameLSN();

    // Check if an entity was hit and the response coordinate system is ENTITY. 
    if (IsValid(entity) && data.eResponseCoordinateSystem == sbio::ETopLevelCoordinateSystem::ENTITY)
    {
      // Send a line of sight extended entity coordinates response with the entity ID and range information.
      sbio::cigi::SLineOfSightExtendedEntityCoordinatesResponse response;
      response.bValid = resValid;
      response.bVisible = !resValid;
      response.bRangeValid = false;
      response.dRange = range;
      response.entityID = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->FindID(entity);
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;
      response.offset = FVectorToCigiBodyCoordinates(resValid ? entity->GetActorTransform().InverseTransformPosition(resHit.Location) : FVector::ZeroVector);
      response.fNormalVectorAzimuth = normal.Z;
      response.fNormalVectorElevation = normal.Y;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightExtendedEntityCoordinatesResponse(response);
      }
    }
    else if (IsValid(entity))
    {
      // If an entity was hit, send a line of sight extended geodetic coordinates response with the entity ID and range information.
      sbio::cigi::SLineOfSightExtendedEntityGeodeticCoordinatesResponse response;
      response.bValid = resValid;
      response.bVisible = !resValid;
      response.bRangeValid = false;
      response.dRange = range;
      response.entityID = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->FindID(entity);
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;
      response.geodeticCoordinates = resValid ? CigiCoordinates::EngineToGeodetic(FUEWorldCoordinates::From(resHit.Location)) : SGeodeticCoordinates();
      response.fNormalVectorAzimuth = normal.Z;
      response.fNormalVectorElevation = normal.Y;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightExtendedEntityGeodeticCoordinatesResponse(response);
      }
    }
    else
    {
      // If no entity was hit, send a line of sight extended geodetic coordinates response with the range and validity information.
      sbio::cigi::SLineOfSightExtendedGeodeticCoordinatesResponse response;
      response.bValid = resValid;
      response.bVisible = !resValid;
      response.bRangeValid = false;
      response.dRange = range;
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;
      response.geodeticCoordinates = resValid ? CigiCoordinates::EngineToGeodetic(FUEWorldCoordinates::From(resHit.Location)) : SGeodeticCoordinates();
      response.fNormalVectorAzimuth = normal.Z;
      response.fNormalVectorElevation = normal.Y;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightExtendedGeodeticCoordinatesResponse(response);
      }
    }
  }
}

void CUnrealCigiTerrainEventHandler::OnLineOfSightVectorRequestBasicMessage(const sbio::ig::terrain::SLineOfSightVectorRequestBasicMessage& data)
{
  // Convert the start and end geocentric coordinates from the CIGI message to Unreal Engine coordinates using the CigiCoordinates utility class.
  const FVector start = CigiCoordinates::GeocentricToEngine(data.Start).ToFVector();
  const FVector end = CigiCoordinates::GeocentricToEngine(data.End).ToFVector();

  TArray<FHitResult> hits;
  TraceTerrain(EventHandler.GetWorld(), start, end, hits);
  TArray<FHitResult> results;
  FilterLineOfSightHits(hits, data.AlphaThreshold, results);
  const int responseCount = results.Num() <= 0 ? 1 : results.Num();

  for (int i = 0; i < responseCount; ++i)
  {
    // Determine if the current hit result is valid and retrieve the corresponding hit information, including the entity and range.
    const bool resValid = results.Num() > i;
    const FHitResult& resHit = resValid ? results[i] : FHitResult();
    ACigiEntity* entity = resValid ? Cast<ACigiEntity>(resHit.GetActor()) : nullptr;
    const double range = resValid ? FVector::Dist(resHit.TraceStart, resHit.Location) / 100.0 : 0.0;
    const uint8 frameLSN = GetCurrentHostFrameLSN();

    if (IsValid(entity))
    {
      // If an entity was hit, send a line of sight entity response with the entity ID and range information.
      sbio::cigi::SLineOfSightEntityResponse response;
      response.bValid = resValid;
      response.bVisible = !resValid;
      response.dRange = range;
      response.entityID = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->FindID(entity);
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightEntityResponse(response);
      }
    }
    else
    {
      // If no entity was hit, send a line of sight response with the range and validity information.
      sbio::cigi::SLineOfSightResponse response;
      response.bValid = resValid;
      response.dRange = range;
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightResponse(response);
      }
    }
  }
}

void CUnrealCigiTerrainEventHandler::OnLineOfSightSegmentRequestExtendedMessage(const sbio::ig::terrain::SLineOfSightSegmentRequestExtendedMessage& data)
{
  // Convert the start and end geocentric coordinates from the CIGI message to Unreal Engine coordinates using the CigiCoordinates utility class. 
  const FVector start = CigiCoordinates::GeocentricToEngine(data.Start).ToFVector();
  const FVector end = CigiCoordinates::GeocentricToEngine(data.End).ToFVector();
  TArray<FHitResult> hits;
  TraceTerrain(EventHandler.GetWorld(), start, end, hits);
  TArray<FHitResult> results;
  FilterLineOfSightHits(hits, data.AlphaThreshold, results);
  const int responseCount = results.Num() <= 0 ? 1 : results.Num();

  // Iterate through each hit result and send the appropriate line of sight response based on the hit information and the requested coordinate system.
  for (int i = 0; i < responseCount; ++i)
  {
    // Determine if the current hit result is valid and retrieve the corresponding hit information, including the entity, normal vector, range, and host frame LSN.
    const bool resValid = results.Num() > i;
    const FHitResult& resHit = resValid ? results[i] : FHitResult();
    ACigiEntity* entity = resValid ? Cast<ACigiEntity>(resHit.GetActor()) : nullptr;
    const FVector normal = resValid ? resHit.Normal.Rotation().Euler() : FVector::ZeroVector;
    const double range = resValid ? FVector::Dist(resHit.TraceStart, resHit.Location) / 100.0 : 0.0;
    const uint8 frameLSN = GetCurrentHostFrameLSN();

    // Check if an entity was hit and the response coordinate system is ENTITY. If so, send a line of sight extended entity coordinates response with the entity ID and offset information.
    if (IsValid(entity) && data.eResponseCoordinateSystem == sbio::ETopLevelCoordinateSystem::ENTITY)
    {
      // If an entity was hit and the response coordinate system is ENTITY, send a line of sight extended entity coordinates response with the entity ID and offset information.
      sbio::cigi::SLineOfSightExtendedEntityCoordinatesResponse response;
      response.bValid = resValid;
      response.bVisible = !resValid;
      response.bRangeValid = false;
      response.dRange = range;
      response.entityID = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->FindID(entity);
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;
      response.offset = FVectorToCigiBodyCoordinates(resValid ? entity->GetActorTransform().InverseTransformPosition(resHit.Location) : FVector::ZeroVector);
      response.fNormalVectorAzimuth = normal.Z;
      response.fNormalVectorElevation = normal.Y;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightExtendedEntityCoordinatesResponse(response);
      }
    }
    else if (IsValid(entity))
    {
      // If an entity was hit, send a line of sight extended entity-geodetic coordinates response with the entity ID and range information.
      sbio::cigi::SLineOfSightExtendedEntityGeodeticCoordinatesResponse response;
      response.bValid = resValid;
      response.bVisible = !resValid;
      response.bRangeValid = false;
      response.dRange = range;
      response.entityID = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->FindID(entity);
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;
      response.geodeticCoordinates = resValid ? CigiCoordinates::EngineToGeodetic(FUEWorldCoordinates::From(resHit.Location)) : SGeodeticCoordinates();
      response.fNormalVectorAzimuth = normal.Z;
      response.fNormalVectorElevation = normal.Y;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightExtendedEntityGeodeticCoordinatesResponse(response);
      }
    }
    else
    {
      // If no entity was hit, send a line of sight extended geodetic coordinates response with the range and validity information.
      sbio::cigi::SLineOfSightExtendedGeodeticCoordinatesResponse response;
      response.bValid = resValid;
      response.bVisible = !resValid;
      response.bRangeValid = false;
      response.dRange = range;
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;
      response.geodeticCoordinates = resValid ? CigiCoordinates::EngineToGeodetic(FUEWorldCoordinates::From(resHit.Location)) : SGeodeticCoordinates();
      response.fNormalVectorAzimuth = normal.Z;
      response.fNormalVectorElevation = normal.Y;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightExtendedGeodeticCoordinatesResponse(response);
      }
    }
  }
}

CUnrealCigiTerrainEventHandler::CUnrealCigiTerrainEventHandler(CUnrealCigiEventHandler& eventHandler) : EventHandler(eventHandler)
{
}

void CUnrealCigiTerrainEventHandler::OnLineOfSightSegmentRequestBasicMessage(const sbio::ig::terrain::SLineOfSightSegmentRequestBasicMessage& data)
{
  // Convert the start and end geocentric coordinates to Unreal Engine coordinates and perform a line trace to detect hits along the line of sight segment.
  const FVector start = CigiCoordinates::GeocentricToEngine(data.Start).ToFVector();
  const FVector end = CigiCoordinates::GeocentricToEngine(data.End).ToFVector();
  TArray<FHitResult> hits;
  TraceTerrain(EventHandler.GetWorld(), start, end, hits);
  DebugLine(EventHandler.GetWorld(), start, end, FColor::Blue);

  // Filter the hits based on the alpha threshold and store the results in a separate array.
  TArray<FHitResult> results;
  FilterLineOfSightHits(hits, data.AlphaThreshold, results);
  const int responseCount = results.Num() <= 0 ? 1 : results.Num();

  // Iterate through the filtered results and send appropriate line of sight responses based on whether an entity was hit or not.
  for (int i = 0; i < responseCount; ++i)
  {
    // Check if the current index is valid and retrieve the corresponding hit result and entity.
    const bool resValid = results.Num() > i;
    const FHitResult& resHit = resValid ? results[i] : FHitResult();
    ACigiEntity* resEntity = resValid ? Cast<ACigiEntity>(resHit.GetActor()) : nullptr;
    DebugLine(EventHandler.GetWorld(), resHit.Location, resHit.Location + (resHit.Normal * 100), FColor::Orange);
    DebugSphere(EventHandler.GetWorld(), resHit.Location, 10.0, FColor::Red);

    const double range = resValid ? FVector::Dist(resHit.TraceStart, resHit.Location) / 100.0 : 0.0;
    const uint8 frameLSN = GetCurrentHostFrameLSN();

    if (IsValid(resEntity))
    {
      // If an entity was hit, send a line of sight entity response with the entity ID and range information.
      sbio::cigi::SLineOfSightEntityResponse response;
      response.bValid = true;
      response.bVisible = !resValid;
      response.dRange = range;
      response.entityID = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->FindID(resEntity);
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;
      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightEntityResponse(response);
      }
    }
    else
    {
      // If no entity was hit, send a basic line of sight response with the range and validity information.
      sbio::cigi::SLineOfSightResponse response;
      response.bValid = true;
      response.dRange = range;
      response.lineOfSightRequestID = data.LosID;
      response.responseCount = static_cast<uint8>(responseCount);
      response.hostFrameLSN = frameLSN;

      if (FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher != nullptr)
      {
        FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendLineOfSightResponse(response);
      }
    }
  }
}

namespace
{
}

bool CUnrealCigiTerrainEventHandler::TraceTerrain(UWorld* world, const FVector& start, const FVector& end, FHitResult& result)
{
  return world != nullptr && world->LineTraceSingleByObjectType(result, start, end, FCollisionObjectQueryParams::AllObjects);
}

bool CUnrealCigiTerrainEventHandler::TraceTerrain(UWorld* world, const FVector& start, const FVector& end, TArray<FHitResult>& results)
{
  return world != nullptr && world->LineTraceMultiByObjectType(results, start, end, FCollisionObjectQueryParams::AllObjects);
}

void CUnrealCigiTerrainEventHandler::FilterLineOfSightHits(const TArray<FHitResult>& hits, double alphaThreshold, TArray<FHitResult>& results)
{
  TSet<ACigiEntity*> foundEntities;

  // Iterate through the hits and filter them based on the alpha threshold and uniqueness of entities
  for (const FHitResult& hit : hits)
  {
    ACigiEntity* entity = Cast<ACigiEntity>(hit.GetActor());

    if (!IsValid(entity))
    {
      results.Add(hit);
    }
    else if (entity->EntityAlpha < alphaThreshold)
    {
      continue;
    }
    else if (!foundEntities.Contains(entity))
    {
      foundEntities.Add(entity);
      results.Add(hit);
    }
  }
}

namespace
{
  uint8 GetCurrentHostFrameLSN()
  {
    sbio::cigi::ig::CCigiImageGenerator* imageGenerator = FUnrealCigi_PluginModule::globals.pImageGenerator.get();

    // If the image generator is not available, return 0 as the host frame LSN.
    if (imageGenerator == nullptr)
    {
      return 0;
    }

    // The host frame LSN is the last host frame number modulo 16, which is equivalent to the last 4 bits of the last host frame number.
    return static_cast<uint8>(imageGenerator->GetLastHostFrameNumber().Value() & 0x0f);
  }
}

void CUnrealCigiTerrainEventHandler::OnHeightAboveTerrainRequestMessage(const sbio::ig::terrain::SHeightAboveTerrainRequestMessage& data)
{
  // Check if the world is valid and if the exported functions event dispatcher is available
  if (!IsValid(EventHandler.GetWorld()) || FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher == nullptr)
  {
    return;
  }

  // Trace a line from a point above the requested geodetic coordinates to a point below it to find the terrain height and normal vector
  const FVector point = CigiCoordinates::GeodeticToEngine(data.Point).ToFVector();
  const FVector traceStart = point + FVector(0.0, 0.0, 1000000.0);
  const FVector traceEnd = point - FVector(0.0, 0.0, 1000000.0);
  FHitResult result;
  const bool bValid = TraceTerrain(EventHandler.GetWorld(), traceStart, traceEnd, result);
  const SGeodeticCoordinates terrainGeodetic = bValid ? CigiCoordinates::EngineToGeodetic(FUEWorldCoordinates::From(result.Location)) : SGeodeticCoordinates();
  const double heightAboveTerrain = bValid ? data.Point.altitude.Value() - terrainGeodetic.altitude.Value() : 0.0;

  if (data.isExtendedRequest)
  {
    // If the request is an extended request, populate the extended response structure
    sbio::cigi::SHATHOTExtendedResponse response;
    response.HATHOTID = data.HatHotID;
    response.bValid = bValid;
    response.hostFrameLSN = GetCurrentHostFrameLSN();
    response.heightAboveTerrain = heightAboveTerrain;

    // If the terrain is valid, populate the height of terrain and normal vector information
    if (bValid)
    {
      response.heightOfTerrain = terrainGeodetic.altitude;
      const FVector normalRot = result.Normal.Rotation().Euler();
      response.normalVectorAzimuth = sbio::math::Degrees180(normalRot.Z);
      response.normalVectorElevation = sbio::math::Degrees90(normalRot.Y);
    }

    FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendHatHotExtendedResponse(response);
  }
  else
  {
    // If the request is not an extended request, populate the basic response structure
    sbio::cigi::SHeightAboveTerrainResponse response;
    response.HATHOTID = data.HatHotID;
    response.bValid = bValid;
    response.hostFrameLSN = GetCurrentHostFrameLSN();
    response.heightAboveTerrain = heightAboveTerrain;
    FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendHeightAboveTerrainResponse(response);
    return;
  }

  
}

void CUnrealCigiTerrainEventHandler::OnHeightOfTerrainRequestMessage(const sbio::ig::terrain::SHeightOfTerrainRequestMessage& data)
{
  // Check if the world is valid and if the exported functions event dispatcher is available
  if (!IsValid(EventHandler.GetWorld()) || FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher == nullptr)
  {
    return;
  }

  // Trace a line from a point above the requested geodetic coordinates to a point below it to find the terrain height and normal vector
  const FVector point = CigiCoordinates::GeodeticToEngine(data.Point).ToFVector();
  const FVector traceStart = point + FVector(0.0, 0.0, 1000000.0);
  const FVector traceEnd = point - FVector(0.0, 0.0, 1000000.0);
  FHitResult result;
  const bool bValid = TraceTerrain(EventHandler.GetWorld(), traceStart, traceEnd, result);
  const SGeodeticCoordinates terrainGeodetic = bValid ? CigiCoordinates::EngineToGeodetic(FUEWorldCoordinates::From(result.Location)) : SGeodeticCoordinates();
  const double heightAboveTerrain = bValid ? data.Point.altitude.Value() - terrainGeodetic.altitude.Value() : 0.0;

  // If the request is not an extended request, populate the basic response structure
  if (data.isExtendedRequest)
  {
    // If the request is an extended request, populate the extended response structure
    sbio::cigi::SHATHOTExtendedResponse response;
    response.HATHOTID = data.HatHotID;
    response.bValid = bValid;
    response.hostFrameLSN = GetCurrentHostFrameLSN();
    response.heightAboveTerrain = heightAboveTerrain;

    // If the terrain is valid, populate the height of terrain and normal vector information  
    if (bValid)
    {
      response.heightOfTerrain = terrainGeodetic.altitude;
      const FVector normalRot = result.Normal.Rotation().Euler();
      response.normalVectorAzimuth = sbio::math::Degrees180(normalRot.Z);
      response.normalVectorElevation = sbio::math::Degrees90(normalRot.Y);
    }

    FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendHatHotExtendedResponse(response);
  }
  else
  {
    // If the request is not an extended request, populate the basic response structure
    sbio::cigi::SHeightOfTerrainResponse response;
    response.HATHOTID = data.HatHotID;
    response.bValid = bValid;
    response.hostFrameLSN = GetCurrentHostFrameLSN();

    // If the terrain is valid, populate the height of terrain information
    if (bValid)
    {
      response.heightOfTerrain = terrainGeodetic.altitude;
    }

    // Send the response using the exported functions event dispatcher
    FUnrealCigi_PluginModule::globals.pExportedFunctionsEventDispatcher->SendHeightOfTerrainResponse(response);
    return;
  }

  
}

void CUnrealCigiTerrainEventHandler::OnTerrestrialSurfaceConditionsChangedMessage()
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("Terrestrial Surface Conditions are not currently supported"));
}

void CUnrealCigiTerrainEventHandler::OnSetRegionalTerrainSurfaceComponentStateMessage(const sbio::ig::terrain::SSetRegionalTerrainSurfaceComponentStateMessage& data)
{
  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::REGIONAL_TERRAIN, data.ComponentID.Value(), data.ComponentState, data.RegionalTerrainSurfaceID.Value(), data.ComponentData);
}

void CUnrealCigiTerrainEventHandler::OnSetGlobalTerrainComponentStateMessage(const sbio::ig::terrain::SSetGlobalTerrainComponentStateMessage& data)
{
  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::GLOBAL_TERRAIN, data.ComponentID.Value(), data.ComponentState, data.GlobalTerrainSurfaceID.Value(), data.ComponentData);
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026