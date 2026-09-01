//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "EngineLib/ImageGeneratorMessages.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;

    /**
     * @class CUnrealCigiEntityEventHandler
     * @brief Handles CIGI entity messages and updates entity actors in the Unreal world.
     */
    class CUnrealCigiEntityEventHandler
    {
    public:
      explicit CUnrealCigiEntityEventHandler(CUnrealCigiEventHandler& eventHandler);
      /**
       * Handle the creation of a new entity.
       *
       * @param data The message data containing information about the entity being created.
       */
      void OnCreateEntityMessage(const sbio::ig::entity::SCreateEntityMessage& data);
      
      /**
       * Handle the update of an entity's top-level transform.
       *
       * @param data The message data containing the updated transform information.
       */
      void OnUpdateTopLevelEntityTransformMessage(const sbio::ig::entity::SUpdateTopLevelEntityTransformMessage& data);
      
      /**
       * Handle the destruction of an entity.
       *
       * @param data The message data containing information about the entity being destroyed.
       */
      void OnDestroyEntityMessage(const sbio::ig::entity::SDestroyEntityMessage& data);
      
      /**
       * Handle the update of a child entity's transform.
       *
       * @param data The message data containing the updated transform information for the child entity.
       */
      void OnUpdateChildEntityTransformMessage(const sbio::ig::entity::SUpdateChildEntityTransformMessage& data);
      
      /**
       * Handle the attachment of an entity to another entity.
       *
       * @param data The message data containing information about the entity attachment.
       */
      void OnSetEntityAttachedMessage(const sbio::ig::entity::SSetEntityAttachedMessage& data);
      
      /**
       * Handle the activation of an entity.
       *
       * @param data The message data containing information about the entity's active state.
       */
      void OnSetEntityActiveMessage(const sbio::ig::entity::SSetEntityActiveMessage& data);
      
      /**
       * Handle the detachment of an entity from another entity.
       *
       * @param data The message data containing information about the entity detachment.
       */
      void OnSetEntityUnattachedMessage(const sbio::ig::entity::SSetEntityUnattachedMessage& data);
      
      /**
       * Handle the update of an entity's component state.
       *
       * @param data The message data containing information about the entity's component state.
       */
      void OnSetEntityComponentStateMessage(const sbio::ig::entity::SSetEntityComponentStateMessage& data);
      
      /**
       * Handle the update of an articulated part's transform.
       *
       * @param data The message data containing the updated transform information for the articulated part.
       */
      void OnUpdateArticulatedPartTransformMessage(const sbio::ig::entity::SUpdateArticulatedPartTransformMessage& data);
      
      /**
       * Handle the visibility setting of an articulated part.
       *
       * @param data The message data containing information about the visibility state of the articulated part.
       */
      void OnSetArticulatedPartVisibleMessage(const sbio::ig::entity::SSetArticulatedPartVisibleMessage& data);
      
      /**
       * Handle the update of an entity's alpha (transparency) value.
       *
       * @param data The message data containing the new alpha value for the entity.
       */
      void OnSetEntityAlphaMessage(const sbio::ig::entity::SSetEntityAlphaMessage& data);
      
      /**
       * Handle the enabling or disabling of collision detection for an entity.
       *
       * @param data The message data containing information about the collision detection state.
       */
      void OnSetEntityCollisionDetectionEnabledMessage(const sbio::ig::entity::SSetEntityCollisionDetectionEnabledMessage& data);

      /**
       * Check if a point is within the volume of an entity.
       *
       * @param point The point in geocentric coordinates to check.
       * @param entityID The ID of the entity to check against.
       * @return True if the point is within the entity's volume, false otherwise.
       */
      bool IsPointInEntityVolume(const sbio::math::GeocentricCoordinates& point, sbio::EntityID entityID) const;

    private:
      CUnrealCigiEventHandler& EventHandler;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026