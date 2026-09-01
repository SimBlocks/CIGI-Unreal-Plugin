//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"
#include "EngineLib/ImageGeneratorMessages.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiEntityManager;

    /**
     * @class CUnrealCigiPhysicsManager
     * @brief Maintains physics-related state for CIGI entity processing.
     */
    class CUnrealCigiPhysicsManager
    {
    public:
      explicit CUnrealCigiPhysicsManager(CUnrealCigiEntityManager& entityManager);

      /**
       * @brief Create a new segment for collision detection.
       * @param data Message data containing segment creation parameters.
       * @return true if the segment was successfully created, false otherwise.
       */
      bool CreateSegment(const sbio::ig::physics::SCreateCollisionDetectionSegmentMessage& data);

      /**
       * @brief Update an existing collision detection segment.
       * @param data Message data containing updated segment parameters.
       * @return true if the segment was successfully updated, false otherwise.
       */
      bool UpdateSegment(const sbio::ig::physics::SSetCollisionDetectionSegmentMessage& data);

      /**
       * @brief Enable or disable a collision detection segment.
       * @param data Message data containing segment ID and enabled state.
       * @return true if the segment enabled state was successfully set, false otherwise.
       */
      bool SetSegmentEnabled(const sbio::ig::physics::SSetCollisionDetectionSegmentEnabledMessage& data);

      /**
       * @brief Update the collision volume of an entity.
       * @param data Message data containing volume update parameters.
       * @return true if the volume was successfully updated, false otherwise.
       */
      bool UpdateVolume(const sbio::ig::physics::SSetCollisionVolumeMessage& data);

      /**
       * @brief Create a spherical collision volume.
       * @param data Message data containing sphere creation parameters.
       * @return true if the sphere volume was successfully created, false otherwise.
       */
      bool CreateVolumeSphere(const sbio::ig::physics::SCreateCollisionVolumeSphereMessage& data);

      /**
       * @brief Create a cuboid collision volume.
       * @param data Message data containing cuboid creation parameters.
       * @return true if the cuboid volume was successfully created, false otherwise.
       */
      bool CreateVolumeCuboid(const sbio::ig::physics::SCreateCollisionVolumeCuboidMessage& data);

      /**
       * @brief Enable or disable a collision volume.
       * @param data Message data containing volume ID and enabled state.
       * @return true if the volume enabled state was successfully set, false otherwise.
       */
      bool SetVolumeEnabled(const sbio::ig::physics::SSetCollisionVolumeEnabledMessage& data);

      /**
       * @brief Destroy a collision volume.
       * @param data Message data containing volume destruction parameters.
       * @return true if the volume was successfully destroyed, false otherwise.
       */
      bool DestroyVolume(const sbio::ig::physics::SDestroyCollisionVolumeMessage& data);

      /**
       * @brief Query if a point is inside an entity's volume.
       * @param point The point in geocentric coordinates to test.
       * @param entityID The ID of the entity to test against.
       * @return true if the point is inside the entity's volume, false otherwise.
       */
      bool IsPointInEntityVolume(const sbio::math::GeocentricCoordinates& point, sbio::EntityID entityID) const;

    private:
      CUnrealCigiEntityManager& EntityManager;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026