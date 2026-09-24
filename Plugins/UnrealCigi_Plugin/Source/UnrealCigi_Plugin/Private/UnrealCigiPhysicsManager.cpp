//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiPhysicsManager.h"
#include "UnrealCigiEntityManager.h"
#include "CigiEntity.h"
#include "UnrealCigiUtil.h"
#include "CigiCoordinates.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

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

      // Check only enabled CIGI collision volumes, not other shapes on the actor
      for (const TPair<int32, UShapeComponent*>& volume : entity->GetCollisionVolumes())
      {
        UShapeComponent* shapeComponent = volume.Value;
        if (!IsValid(shapeComponent) || shapeComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
        {
          continue;
        }

        if (const USphereComponent* sphere = Cast<USphereComponent>(shapeComponent))
        {
          const FVector componentPoint = sphere->GetComponentQuat().UnrotateVector(enginePoint - sphere->GetComponentLocation());
          const float radius = sphere->GetScaledSphereRadius();
          if (componentPoint.SizeSquared() <= FMath::Square(radius))
          {
            return true;
          }
        }
        else if (const UBoxComponent* box = Cast<UBoxComponent>(shapeComponent))
        {
          const FVector localPoint = box->GetComponentTransform().InverseTransformPosition(enginePoint);
          const FVector extent = box->GetUnscaledBoxExtent();
          if (FMath::Abs(localPoint.X) <= extent.X && FMath::Abs(localPoint.Y) <= extent.Y && FMath::Abs(localPoint.Z) <= extent.Z)
          {
            return true;
          }
        }
        else if (const UCapsuleComponent* capsule = Cast<UCapsuleComponent>(shapeComponent))
        {
          float radius;
          float halfHeight;
          capsule->GetScaledCapsuleSize(radius, halfHeight);
          const FVector componentPoint = capsule->GetComponentQuat().UnrotateVector(enginePoint - capsule->GetComponentLocation());
          const float segmentHalfLength = FMath::Max(0.0f, halfHeight - radius);
          const float closestZ = FMath::Clamp(componentPoint.Z, -segmentHalfLength, segmentHalfLength);
          const FVector closestPoint(0.0f, 0.0f, closestZ);
          if (FVector::DistSquared(componentPoint, closestPoint) <= FMath::Square(radius))
          {
            return true;
          }
        }
      }

      return false;
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026