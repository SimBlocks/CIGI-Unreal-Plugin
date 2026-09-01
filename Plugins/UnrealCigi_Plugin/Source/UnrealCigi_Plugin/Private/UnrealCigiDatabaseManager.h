//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelStreamingDynamic.h"

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @struct DatabaseInfo
     * @brief Stores the database metadata maintained by the database manager.
     */
    struct DatabaseInfo
    {
      /** Asset path used to load the database level. */
      FString assetPath;
      /** Streaming level instance for the database. */
      ULevelStreamingDynamic* levelInstance = nullptr;
      /** Geodetic or world origin associated with the database. */
      FVector origin = FVector::ZeroVector;
    };
  }
}

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CUnrealCigiDatabaseManager
     * @brief Maintains the active CIGI database state used by the plugin.
     */
    class CUnrealCigiDatabaseManager
    {
    public:
      /**
       * @brief Constructs a database manager.
       * @param world Unreal world used for database level operations, or nullptr.
       */
      explicit CUnrealCigiDatabaseManager(UWorld* world = nullptr);

      /**
       * @brief Sets the Unreal world used for database level operations.
       * @param world Unreal world to associate with the manager.
       */
      void SetWorld(UWorld* world);

      /**
       * @brief Clears tracked database state and unloads managed database resources.
       */
      void Reset();

      /**
       * @brief Processes a level-added event for database tracking.
       * @param level Level that was added to the world.
       * @param world World to which the level was added.
       */
      void OnLevelAddedToWorld(ULevel* level, UWorld* world);
      /**
       * @brief Finds a database by its ID.
       * @param databaseID The ID of the database to find.
       * @return Pointer to the DatabaseInfo if found, nullptr otherwise.
       */
      DatabaseInfo* Find(int32 databaseID);
      /**
       * @brief Finds a database by its ID (const version).
       * @param databaseID The ID of the database to find.
       * @return Pointer to the DatabaseInfo if found, nullptr otherwise.
       */
      const DatabaseInfo* Find(int32 databaseID) const;

      /** Unreal world used for database level operations. */
      UWorld* World = nullptr;
      /** Database metadata indexed by database ID. */
      TMap<int32, DatabaseInfo> Databases;
      /** Currently active streamed database level. */
      ULevelStreamingDynamic* CurrentLevel = nullptr;
      /** Whether a database is currently loaded. */
      bool bLoaded = false;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026