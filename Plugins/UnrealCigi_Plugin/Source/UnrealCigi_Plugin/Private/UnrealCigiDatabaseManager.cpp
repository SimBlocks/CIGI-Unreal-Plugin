//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiDatabaseManager.h"
#include "UnrealCigi_PluginPrivate.h"
#include "CigiCoordinates.h"

namespace sbio
{
  namespace unrealcigi
  {
    CUnrealCigiDatabaseManager::CUnrealCigiDatabaseManager(UWorld* world) : World(world)
    {
    }

    void CUnrealCigiDatabaseManager::OnLevelAddedToWorld(ULevel* level, UWorld* world)
    {
      // Ensure that the level added event is for the correct world
      if (world != World)
      {
        return;
      }

      // Update the CigiCoordinates system based on the newly added level
      CigiCoordinates::TryFindGeoReferencingSystem();
      if (!IsValid(CurrentLevel) || CurrentLevel->GetLoadedLevel() != level)
      {
        UE_LOG(LogCigiEventHandler, Log, TEXT("OnLevelAddedToWorld: Ignoring unrelated streamed level '%s'"), IsValid(level) ? *level->GetName() : TEXT("NULL"));
        return;
      }

      bLoaded = true;
      sbio::cigi::ig::SDatabaseLoadedEventArgs args;
      args.bLoadSuccessful = true;
      sbio::utils::Event::Raise<sbio::cigi::ig::IGCIGIEvent>(args);
      UE_LOG(LogCigiEventHandler, Log, TEXT("OnLevelAddedToWorld: Confirmed database level '%s' loaded"), IsValid(level) ? *level->GetName() : TEXT("NULL"));
    }

    void CUnrealCigiDatabaseManager::SetWorld(UWorld* world)
    {
      World = world;
    }

    void CUnrealCigiDatabaseManager::Reset()
    {
      Databases.Empty();
      CurrentLevel = nullptr;
      bLoaded = false;
    }

    DatabaseInfo* CUnrealCigiDatabaseManager::Find(int32 databaseID)
    {
      return Databases.Find(databaseID);
    }

    const DatabaseInfo* CUnrealCigiDatabaseManager::Find(int32 databaseID) const
    {
      return Databases.Find(databaseID);
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026