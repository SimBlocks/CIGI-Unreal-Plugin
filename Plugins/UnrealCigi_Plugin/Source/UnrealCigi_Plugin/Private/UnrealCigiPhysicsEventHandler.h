//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "EngineLib/ImageGeneratorMessages.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEventHandler;

    /**
     * @class CUnrealCigiPhysicsEventHandler
     * @brief Handles CIGI physics messages and applies their effects to Unreal entities.
     */
    class CUnrealCigiPhysicsEventHandler
    {
    public:
      /**
       * @brief Construct a new CUnrealCigiPhysicsEventHandler object.
       *
       * @param eventHandler Reference to the associated event handler.
       */
      explicit CUnrealCigiPhysicsEventHandler(CUnrealCigiEventHandler& eventHandler);

      /**
       * @brief Handle the creation of a collision detection segment.
       *
       * @param data Message containing data about the collision detection segment to be created.
       */
      void OnCreateCollisionDetectionSegmentMessage(const sbio::ig::physics::SCreateCollisionDetectionSegmentMessage& data);

      /**
       * @brief Handle the setting of a collision detection segment.
       *
       * @param data Message containing data about the collision detection segment to be set.
       */
      void OnSetCollisionDetectionSegmentMessage(const sbio::ig::physics::SSetCollisionDetectionSegmentMessage& data);

      /**
       * @brief Enable or disable a collision detection segment.
       *
       * @param data Message containing data about the collision detection segment and its new enabled state.
       */
      void OnSetCollisionDetectionSegmentEnabledMessage(const sbio::ig::physics::SSetCollisionDetectionSegmentEnabledMessage& data);

      /**
       * @brief Set the collision volume for an entity.
       *
       * @param data Message containing data about the collision volume to be set.
       */
      void OnSetCollisionVolumeMessage(const sbio::ig::physics::SSetCollisionVolumeMessage& data);

      /**
       * @brief Create a spherical collision volume.
       *
       * @param data Message containing data about the spherical collision volume to be created.
       */
      void OnCreateCollisionVolumeSphereMessage(const sbio::ig::physics::SCreateCollisionVolumeSphereMessage& data);

      /**
       * @brief Create a cuboidal collision volume.
       *
       * @param data Message containing data about the cuboidal collision volume to be created.
       */
      void OnCreateCollisionVolumeCuboidMessage(const sbio::ig::physics::SCreateCollisionVolumeCuboidMessage& data);

      /**
       * @brief Enable or disable a collision volume.
       *
       * @param data Message containing data about the collision volume and its new enabled state.
       */
      void OnSetCollisionVolumeEnabledMessage(const sbio::ig::physics::SSetCollisionVolumeEnabledMessage& data);

      /**
       * @brief Destroy a collision volume.
       *
       * @param data Message containing data about the collision volume to be destroyed.
       */
      void OnDestroyCollisionVolumeMessage(const sbio::ig::physics::SDestroyCollisionVolumeMessage& data);

    private:
      CUnrealCigiEventHandler& EventHandler; ///< Reference to the associated event handler.
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026