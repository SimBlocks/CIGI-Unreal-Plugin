//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"
#include "EntityConfig.h"

class ACigiEntity;

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CUnrealCigiEntityManager
     * @brief Tracks the CIGI entity actors managed by the plugin.
     */
    class CUnrealCigiEntityManager
    {
    public:
      explicit CUnrealCigiEntityManager(UWorld* world = nullptr);

      /**
       * @brief Set the world context.
       * @param world - Pointer to the world context.
       */
      void SetWorld(UWorld* world);

      /**
       * @brief Reset the manager, clearing all entities and configs.
       */
      void Reset();

      /**
       * @brief Release all entity configs.
       */
      void ReleaseConfigs();

      /**
       * @brief Load blueprints for entities.
       */
      void LoadBlueprints();

      /**
       * @brief Find an entity by its ID.
       * @param entityID - The ID of the entity to find.
       * @return Pointer to the found ACigiEntity, or nullptr if not found.
       */
      ACigiEntity* Find(sbio::EntityID entityID) const;

      /**
       * @brief Find the ID of an entity.
       * @param entity - Pointer to the ACigiEntity.
       * @return The ID of the entity.
       */
      sbio::EntityID FindID(const ACigiEntity* entity) const;

      /**
       * @brief Attach one entity to another.
       * @param entityID - The ID of the entity to attach.
       * @param parentID - The ID of the parent entity.
       * @return true if the attachment was successful, false otherwise.
       */
      bool Attach(sbio::EntityID entityID, sbio::EntityID parentID);

      /**
       * @brief Set the active state of an entity.
       * @param entityID - The ID of the entity.
       * @param active - true to activate the entity, false to deactivate.
       * @return true if the operation was successful, false otherwise.
       */
      bool SetActive(sbio::EntityID entityID, bool active);

      /**
       * @brief Detach an entity from its parent.
       * @param entityID - The ID of the entity to detach.
       * @return true if the detachment was successful, false otherwise.
       */
      bool Detach(sbio::EntityID entityID);

      /**
       * @brief Set the alpha (transparency) of an entity.
       * @param entityID - The ID of the entity.
       * @param alpha - The alpha value (0.0 to 1.0).
       * @return true if the operation was successful, false otherwise.
       */
      bool SetAlpha(sbio::EntityID entityID, float alpha);

      /**
       * @brief Enable or disable collision for an entity.
       * @param entityID - The ID of the entity.
       * @param enabled - true to enable collision, false to disable.
       * @return true if the operation was successful, false otherwise.
       */
      bool SetCollisionEnabled(sbio::EntityID entityID, bool enabled);

      /** World context pointer. */
      UWorld* World = nullptr;

      /** Map of actor IDs to ACigiEntity pointers. */
      TMap<uint32, ACigiEntity*> Actors;

      /** Map of entity config names to UEntityConfig pointers. */
      TMap<FString, UEntityConfig*> Configs;

      /** Map of short type IDs to extended type names. */
      TMap<int32, FString> ShortTypeToExtended;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026