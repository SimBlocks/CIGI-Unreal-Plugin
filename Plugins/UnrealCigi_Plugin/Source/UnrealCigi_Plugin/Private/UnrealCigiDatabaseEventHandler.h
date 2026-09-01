//Copyright SimBlocks LLC 2016-2026
#pragma once

class ULevel;
class UWorld;

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiCelestialEventHandler;

    /**
     * @class CUnrealCigiDatabaseEventHandler
     * @brief Handles CIGI database events and applies them to the Unreal scene.
     */
    class CUnrealCigiDatabaseEventHandler
    {
    public:
      /**
       * @brief Forwards a level-added notification to the database manager.
       *
       * The database manager uses the notification to detect completion of the
       * current streamed database load and raise the corresponding loaded event.
       *
       * @param level Level that was added to the world.
       * @param world World to which the level was added.
       */
      void OnLevelAddedToWorld(ULevel* level, UWorld* world);

      /**
       * @brief Unloads the current database and resets its world state.
       *
       * Disables loading of the current streamed level, clears the database manager's
       * active state, resets the geodetic origin, and updates the celestial coordinates.
       * If no database or level instance is loaded, the function has no effect.
       *
       * @param world World associated with the current database.
       * @param celestialHandler Handler to update after the geodetic origin is reset.
       */
      void OnUnloadDatabase(UWorld* world, CUnrealCigiCelestialEventHandler& celestialHandler);

      /**
       * @brief Loads the registered database with the specified identifier.
       *
       * Unloads any previous database when necessary, applies the registered geodetic
       * origin, updates celestial coordinates, and loads or reuses the database's
       * streaming level. A database whose asset path is "none" applies its origin
       * without loading an Unreal level. Load success or failure is reported through
       * the CIGI event system.
       *
       * @param databaseID Identifier of the registered database to load.
       * @param world World in which to load the database level.
       * @param celestialHandler Handler to update with the database's coordinates.
       */
      void LoadDatabase(uint32 databaseID, UWorld* world, CUnrealCigiCelestialEventHandler& celestialHandler);
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026