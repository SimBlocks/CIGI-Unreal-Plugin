//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiDatabaseEventHandler.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiDatabaseManager.h"
#include "unrealcigiCelestialEventHandler.h"
#include "CigiCoordinates.h"
#include "Engine/LevelStreamingDynamic.h"
#include "unrealcigiUtil.h"

namespace sbio
{
  namespace unrealcigi
  {
    using namespace utils;

    void CUnrealCigiDatabaseEventHandler::OnLevelAddedToWorld(ULevel* level, UWorld* world)
    {
      FUnrealCigi_PluginModule::globals.pDatabaseManager->OnLevelAddedToWorld(level, world);
    }

    void CUnrealCigiDatabaseEventHandler::OnUnloadDatabase(UWorld* world, CUnrealCigiCelestialEventHandler& celestialHandler)
    {
      // Check if a database is currently loaded or if there is a level instance that should be loaded. 
      // If neither condition is true, log a message and return early.
      const bool bHasLoadedLevelInstance = IsValid(FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel) && FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel->ShouldBeLoaded();
      if (!FUnrealCigi_PluginModule::globals.pDatabaseManager->bLoaded && !bHasLoadedLevelInstance)
      {
        UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUnloadDatabaseMessage: No database has currently been loaded."));
        return;
      }

      // If a level instance is currently loaded, mark it as not loaded so that it can be unloaded.
      if (bHasLoadedLevelInstance)
      {
        FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel->SetShouldBeLoaded(false);
      }

      // Reset the database manager state
      FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel = nullptr;
      FUnrealCigi_PluginModule::globals.pDatabaseManager->bLoaded = false;
      CigiCoordinates::SetGeodeticOrigin(SGeodeticCoordinates());
      celestialHandler.UpdateLatLon(Latitude(0.0), Longitude(0.0), world);
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnUnloadDatabaseMessage: Unloaded current database"));
    }

    void CUnrealCigiDatabaseEventHandler::LoadDatabase(uint32 databaseID, UWorld* world, CUnrealCigiCelestialEventHandler& celestialHandler)
    {
      // Check if the database manager is available
      auto p_database = FUnrealCigi_PluginModule::globals.pDatabaseManager->Databases.Find(databaseID);
      if (p_database == nullptr)
      {
        UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnLoadDatabaseMessage: FAILED: no registered database with id %d!"), databaseID);
        return;
      }

      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnLoadDatabaseMessage: Loading database with id=%d, path='%s', instance=%d"), databaseID, *(p_database->assetPath), (p_database->levelInstance == nullptr ? 0 : 1));

      bool bOriginApplied = false;
      bool assetPathNone = p_database->assetPath.Equals(TEXT("none"), ESearchCase::IgnoreCase);
      const bool bHasPreviousLevelInstance = IsValid(FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel) && FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel->ShouldBeLoaded();
      const bool bReusingCurrentLevelInstance = IsValid(FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel) && FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel == p_database->levelInstance;

      // If a database is already loaded or if there is a previous level instance, we need to unload it before loading the new one. 
      // However, if reusing the current level instance, don't unload it.
      if ((FUnrealCigi_PluginModule::globals.pDatabaseManager->bLoaded || bHasPreviousLevelInstance) && !bReusingCurrentLevelInstance)
      {
        OnUnloadDatabase(world, celestialHandler);
      }
      else
      {
        FUnrealCigi_PluginModule::globals.pDatabaseManager->bLoaded = false;
      }

      // If reusing the current level instance, don't set CurrentLevel to nullptr, as it is already set to the correct level instance.
      if (!bReusingCurrentLevelInstance)
      {
        FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel = nullptr;
      }

      // If the asset path is "none", don't load any level, but still set the geodetic origin and mark the database as loaded.
      bool bDatabaseLoadCompleted = false;
      if (assetPathNone)
      {
        UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnLoadDatabaseMessage: No filepath was provided for database id=%d, so no Unreal Level will be loaded."), databaseID);
        CigiCoordinates::SetGeodeticOrigin(SGeodeticCoordinates(Latitude(p_database->origin.X), Longitude(p_database->origin.Y), p_database->origin.Z));
        bOriginApplied = true;
        bDatabaseLoadCompleted = true;
        FUnrealCigi_PluginModule::globals.pDatabaseManager->bLoaded = true;
        sbio::cigi::ig::SDatabaseLoadedEventArgs args;
        args.bLoadSuccessful = true;
        sbio::utils::Event::Raise<sbio::cigi::ig::IGCIGIEvent>(args);
      }
      else
      {
        if (p_database->levelInstance != nullptr && IsValid(p_database->levelInstance))
        {
          FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel = p_database->levelInstance;
          if (p_database->levelInstance->GetLoadedLevel() != nullptr)
          {
            CigiCoordinates::SetGeodeticOrigin(SGeodeticCoordinates(Latitude(p_database->origin.X), Longitude(p_database->origin.Y), p_database->origin.Z));
            bOriginApplied = true;
            bDatabaseLoadCompleted = true;
            FUnrealCigi_PluginModule::globals.pDatabaseManager->bLoaded = true;
            sbio::cigi::ig::SDatabaseLoadedEventArgs args;
            args.bLoadSuccessful = true;
            sbio::utils::Event::Raise<sbio::cigi::ig::IGCIGIEvent>(args);
            UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnLoadDatabaseMessage: Database id=%d, path='%s' has an existing instance that is already loaded."), databaseID, *p_database->assetPath);
          }
          else
          {
            p_database->levelInstance->SetShouldBeLoaded(true);
            UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnLoadDatabaseMessage: Loading the pre-existing level instance for database id=%d, path='%s'"), databaseID, *p_database->assetPath);
          }
        }
        else
        {
          bool outSuccess;
          auto newLevel = ULevelStreamingDynamic::LoadLevelInstance(world, p_database->assetPath, FVector(0, 0, 0), FRotator::MakeFromEuler(FVector(0, 0, 0)), outSuccess);
          if (!outSuccess)
          {
            UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnLoadDatabaseMessage: FAILED: database id=%d failed to load! Could not find UMAP file at path='%s'"), databaseID, *p_database->assetPath);
            sbio::cigi::ig::SDatabaseLoadedEventArgs args;
            args.bLoadSuccessful = false;
            sbio::utils::Event::Raise<sbio::cigi::ig::IGCIGIEvent>(args);
            return;
          }
          p_database->levelInstance = newLevel;
          FUnrealCigi_PluginModule::globals.pDatabaseManager->CurrentLevel = p_database->levelInstance;
          newLevel->SetShouldBeLoaded(true);
        }
      }

      // If the geodetic origin has not been applied yet, set it using the database's origin coordinates
      if (!bOriginApplied)
      {
        CigiCoordinates::SetGeodeticOrigin(SGeodeticCoordinates(Latitude(p_database->origin.X), Longitude(p_database->origin.Y), p_database->origin.Z));
      }

      // Log the database load event with the geodetic origin and level name
      FString debugLevelName = TEXT("ERROR");
      if (IsValid(p_database->levelInstance))
      {
        debugLevelName = *ObjName(p_database->levelInstance);
      }
      else if (assetPathNone)
      {
        debugLevelName = TEXT("NONE");
      }

      // Update the celestial handler with the geodetic origin
      SGeodeticCoordinates geodeticOrigin = CigiCoordinates::GetGeodeticOrigin();
      celestialHandler.UpdateLatLon(geodeticOrigin.latitude, geodeticOrigin.longitude, world);
      FUnrealCigi_PluginModule::globals.pDatabaseManager->bLoaded = bDatabaseLoadCompleted;
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("OnLoadDatabaseMessage: Loading database {id=%d, origin=[%.2f,%.2f,%.2f], path=\"%s\"} with display name=\"%s\""), databaseID, geodeticOrigin.latitude.Value(), geodeticOrigin.longitude.Value(), geodeticOrigin.altitude.Value(), *p_database->assetPath,
             *debugLevelName);
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026