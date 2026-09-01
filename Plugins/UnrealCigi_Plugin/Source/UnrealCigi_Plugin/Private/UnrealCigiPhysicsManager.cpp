//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiPhysicsManager.h"
#include "UnrealCigiEntityManager.h"
#include "CigiEntity.h"
#include "UnrealCigiUtil.h"
#include "CigiCoordinates.h"
#include "Components/ShapeComponent.h"

namespace sbio
{
  namespace unrealcigi
  {
    CUnrealCigiPhysicsManager::CUnrealCigiPhysicsManager(CUnrealCigiEntityManager& entityManager) : EntityManager(entityManager)
    {
    }

    bool CUnrealCigiPhysicsManager::CreateSegment(const sbio::ig::physics::SCreateCollisionDetectionSegmentMessage& data)
    {
      // Find the entity associated with the provided EntityID
      ACigiEntity* entity = EntityManager.Find(data.EntityID);

      // Check if the entity is valid
      if (!IsValid(entity))
      {
        return false;
      }

      // Create the collision segment with the provided SegmentID
      return entity->CreateCollisionSegment(data.SegmentID);
    }

    bool CUnrealCigiPhysicsManager::UpdateSegment(const sbio::ig::physics::SSetCollisionDetectionSegmentMessage& data)
    {
      // Find the entity associated with the provided EntityID
      ACigiEntity* entity = EntityManager.Find(data.EntityID);

      // Check if the entity is valid
      if (!IsValid(entity))
      {
        return false;
      }

      // Update the collision segment with the provided BeginPos and EndPos, converting them to Unreal Engine coordinates
      return entity->UpdateCollisionSegment(data.SegmentID, utils::BodyCoordinatesToFVector(data.BeginPos), utils::BodyCoordinatesToFVector(data.EndPos));
    }

    bool CUnrealCigiPhysicsManager::SetSegmentEnabled(const sbio::ig::physics::SSetCollisionDetectionSegmentEnabledMessage& data)
    {
      // Find the entity associated with the provided EntityID
      ACigiEntity* entity = EntityManager.Find(data.EntityID);

      // Check if the entity is valid
      if (!IsValid(entity))
      {
        return false;
      }

      // Update the enabled state of the collision segment
      return entity->UpdateCollisionSegment(data.SegmentID, data.Enabled);
    }

    bool CUnrealCigiPhysicsManager::UpdateVolume(const sbio::ig::physics::SSetCollisionVolumeMessage& data)
    {
      // Find the entity associated with the provided EntityID
      ACigiEntity* entity = EntityManager.Find(data.EntityID);

      // Check if the entity is valid
      if (!IsValid(entity))
      {
        return false;
      }

      bool sizeSet = false;

      // Set the size of the collision volume based on the provided data
      if (data.Radius > 0)
      {
        sizeSet = entity->SetCollisionVolumeSize(data.VolumeID, FVector::OneVector * data.Radius * 100.0f);
      }
      else if (data.Depth > 0 || data.Width > 0 || data.Height > 0)
      {
        sizeSet = entity->SetCollisionVolumeSize(data.VolumeID, FVector(data.Depth, data.Width, data.Height) * 100.0f);
      }

      // Set the offset and rotation of the collision volume
      const bool offsetSet = entity->SetCollisionVolumeOffset(data.VolumeID, utils::BodyCoordinatesToFVector(data.Offset));
      const FQuat rotation = FRotationMatrix::MakeFromXZ(utils::BodyCoordinatesToFVector(data.Rotation.Forward), utils::BodyCoordinatesToFVector(data.Rotation.Up)).ToQuat();
      const bool rotationSet = entity->SetCollisionVolumeRotation(data.VolumeID, rotation);

      // Return true if any of the size, offset, or rotation was successfully set
      return sizeSet || offsetSet || rotationSet;
    }

    bool CUnrealCigiPhysicsManager::CreateVolumeSphere(const sbio::ig::physics::SCreateCollisionVolumeSphereMessage& data)
    {
      ACigiEntity* entity = EntityManager.Find(data.EntityID);
      return IsValid(entity) && entity->CreateCollisionVolume(data.VolumeID, true);
    }

    bool CUnrealCigiPhysicsManager::CreateVolumeCuboid(const sbio::ig::physics::SCreateCollisionVolumeCuboidMessage& data)
    {
      ACigiEntity* entity = EntityManager.Find(data.EntityID);
      return IsValid(entity) && entity->CreateCollisionVolume(data.VolumeID, false);
    }

    bool CUnrealCigiPhysicsManager::SetVolumeEnabled(const sbio::ig::physics::SSetCollisionVolumeEnabledMessage& data)
    {
      ACigiEntity* entity = EntityManager.Find(data.EntityID);
      return IsValid(entity) && entity->SetCollisionVolumeEnabled(data.VolumeID, data.Enabled);
    }

    bool CUnrealCigiPhysicsManager::DestroyVolume(const sbio::ig::physics::SDestroyCollisionVolumeMessage& data)
    {
      ACigiEntity* entity = EntityManager.Find(data.EntityID);
      return IsValid(entity) && entity->DestroyCollisionVolume(data.VolumeID);
    }

    bool CUnrealCigiPhysicsManager::IsPointInEntityVolume(const sbio::math::GeocentricCoordinates& point, sbio::EntityID entityID) const
    {
      ACigiEntity* entity = EntityManager.Find(entityID);

      // Check if the entity is valid
      if (!IsValid(entity))
      {
        return false;
      }

      // Convert the geocentric point to Unreal Engine coordinates
      const FVector enginePoint = CigiCoordinates::GeocentricToEngine(point).ToFVector();
      TArray<UShapeComponent*> shapeComponents;
      entity->GetComponents<UShapeComponent>(shapeComponents);

      // Check if the point is inside any of the shape components
      for (UShapeComponent* shapeComponent : shapeComponents)
      {
        if (IsValid(shapeComponent) && shapeComponent->Bounds.GetBox().IsInsideOrOn(enginePoint))
        {
          return true;
        }
      }

      return false;
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026