//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @brief Strongly typed wrapper for Unreal world-space coordinates.
     *
     * This type keeps Unreal world/reference-plane positions distinct from other
     * coordinate-system vectors while still allowing an explicit conversion to
     * `FVector` at the Unreal API boundary.
     */
    struct FUEWorldCoordinates
    {
      /** Constructs coordinates initialized to the zero vector. */
      FUEWorldCoordinates() = default;

      /**
       * @brief Constructs coordinates from an Unreal vector.
       * @param InVector Source world-space vector.
       */
      explicit FUEWorldCoordinates(const FVector& InVector);

      /**
       * @brief Constructs coordinates from Cartesian components.
       * @param InX X component.
       * @param InY Y component.
       * @param InZ Z component.
       */
      explicit FUEWorldCoordinates(float InX, float InY, float InZ);

      template <typename TVector>
      /**
       * @brief Creates coordinates from a vector type convertible to FVector.
       * @param InVector Source vector.
       * @return Strongly typed Unreal world coordinates.
       */
      static FUEWorldCoordinates From(const TVector& InVector)
      {
        return FUEWorldCoordinates(FVector(InVector));
      }

      /**
       * @brief Converts these coordinates to FVector.
       * @return Unreal world-space vector.
       */
      FVector ToFVector() const;

    private:
      /** Stored Unreal world-space vector. */
      FVector Value = FVector::ZeroVector;
    };

    /**
     * @brief Strongly typed wrapper for Unreal world-space rotation.
     *
     * This type keeps Unreal world rotations distinct from other rotation
     * conventions while still allowing explicit conversion to `FQuat` at the
     * Unreal API boundary.
     */
    struct FUEWorldRotation
    {
      /** Constructs a rotation initialized to identity. */
      FUEWorldRotation() = default;

      /**
       * @brief Constructs a rotation from a quaternion.
       * @param InRotation Source quaternion.
       */
      explicit FUEWorldRotation(const FQuat& InRotation);
      /**
       * @brief Constructs a rotation from an Unreal rotator.
       * @param InRotation Source rotator.
       */
      explicit FUEWorldRotation(const FRotator& InRotation);

      template <typename TRotation>
      /**
       * @brief Creates a rotation from a type convertible to FQuat.
       * @param InRotation Source rotation.
       * @return Strongly typed Unreal world rotation.
       */
      static FUEWorldRotation From(const TRotation& InRotation)
      {
        return FUEWorldRotation(FQuat(InRotation));
      }

      /**
       * @brief Converts this rotation to a quaternion.
       * @return Unreal quaternion.
       */
      FQuat ToFQuat() const;
      /**
       * @brief Converts this rotation to a rotator.
       * @return Unreal rotator.
       */
      FRotator ToFRotator() const;

    private:
      /** Stored Unreal world-space quaternion. */
      FQuat Value = FQuat::Identity;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026