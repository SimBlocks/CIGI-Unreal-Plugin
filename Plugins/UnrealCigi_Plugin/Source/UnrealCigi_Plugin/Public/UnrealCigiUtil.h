//Copyright SimBlocks LLC 2016-2026
/**
 * @file unrealcigiUtil.h
 * @brief Utility functions, constants, and logging macros for SimBlocks UnrealCIGI plugin.
 *
 * This header provides:
 * - Logging macros for controlling debug output across plugin subsystems.
 * - Mathematical and configuration constants for geodetic conversions, view projections, and debug drawing.
 * - Utility functions for coordinate conversions, asset management, string formatting, debug drawing, and asset discovery.
 *
 * Usage:
 * - Use the provided macros to control log verbosity for different plugin features.
 * - Use conversion functions to translate between Unreal and CIGI coordinate systems.
 * - Use utility functions for safe pointer dereferencing, object naming, string formatting, and debug visualization.
 * - Use asset discovery functions to find Blueprint and font assets in the project.
 */

#pragma once

#include "CoreMinimal.h"
#include "EngineLib/IImageGeneratorEventMessenger.h"
#include "Engine/Font.h"
#include "CigiLib/CigiTypes.h"
#include "UnrealCoordinates.h"

// --- Logging Macros ---
/** @def SLATE_VERTS
 *  @brief Verbosity for Slate vertex debug output. */
#define SLATE_VERTS Verbose
/** @def SLATE_SYMBOLS
 *  @brief Verbosity for Slate symbol debug output. */
#define SLATE_SYMBOLS Verbose
/** @def CIGI_VELOCITY
 *  @brief Verbosity for CIGI velocity debug output. */
#define CIGI_VELOCITY Verbose
/** @def ENTITY_TICK
 *  @brief Verbosity for entity tick debug output. */
#define ENTITY_TICK Verbose
/** @def JSON_LOG
 *  @brief Verbosity for JSON log output. */
#define JSON_LOG Display
/** @def BP_LOG
 *  @brief Verbosity for Blueprint log output. */
#define BP_LOG Display
/** @def CIGI_LOG
 *  @brief Verbosity for CIGI log output. */
#define CIGI_LOG Display
/** @def DBMSG_LOG
 *  @brief Verbosity for database message log output. */
#define DBMSG_LOG Display
/** @def ENTITY_LOG
 *  @brief Verbosity for entity log output. */
#define ENTITY_LOG Display
/** @def JSON_WARNING
 *  @brief Verbosity for JSON warning output. */
#define JSON_WARNING Warning
/** @def BP_WARNING
 *  @brief Verbosity for Blueprint warning output. */
#define BP_WARNING Warning
/** @def CIGI_WARNING
 *  @brief Verbosity for CIGI warning output. */
#define CIGI_WARNING Warning
/** @def ENTITY_WARNING
 *  @brief Verbosity for entity warning output. */
#define ENTITY_WARNING Warning

DECLARE_LOG_CATEGORY_EXTERN(LogCigiUtil, Log, All)

// Forward declaration to avoid header loop
class ACigiEntity;

/**
 * @namespace sbio::unrealcigi::utils
 * @brief Utility namespace for SimBlocks UnrealCIGI plugin.
 */
namespace sbio
{
  namespace unrealcigi
  {
    namespace utils
    {
      /** Root filepath for imported content (Blueprints, Maps). */
      const FString IMPORTED_CONTENT = "/Game";

      /** Default projection mode. */
      const sbio::EProjectionMode DEFAULT_VIEWPROJ_PROJMODE = sbio::EProjectionMode::PERSPECTIVE;
      /** Default mirror mode. */
      const sbio::EMirrorMode DEFAULT_VIEWPROJ_MIRROR = sbio::EMirrorMode::NONE;
      /** Default near clipping distance. */
      const int DEFAULT_VIEWPROJ_NEAR = 1;
      /** Default far clipping distance. */
      const int DEFAULT_VIEWPROJ_FAR = 10000;
      /** Default left/right projection extent. */
      const int DEFAULT_VIEWPROJ_LR = 40;
      /** Default top/bottom projection extent. */
      const int DEFAULT_VIEWPROJ_TB = 30;

      /** Enable debug drawing (lines, spheres, etc.) in the level (1=enable, 0=disable). */
      const int ENABLE_DBG_DRAW = 0;
      /** Debug draw line/sphere thickness. */
      const int DBG_DRAW_THICKNESS = 10;

      /**
       * @brief Converts an Unreal FVector to CIGI BodyCoordinates.
       * @param fv Unreal FVector.
       * @return CIGI BodyCoordinates struct.
       */
      sbio::math::BodyCoordinates FVectorToBodyCoordinates(FVector fv);

      /**
       * @brief Converts CIGI BodyCoordinates to an Unreal FVector.
       * @param vec3Body CIGI BodyCoordinates struct.
       * @return Unreal FVector.
       */
      FVector BodyCoordinatesToFVector(sbio::math::BodyCoordinates vec3Body);

      /**
       * @brief Converts a CIGI body rotation to an Unreal quaternion.
       * @param bodyRot CIGI body rotation.
       * @return Unreal quaternion representing the rotation.
       */
      FQuat BodyRotationToFQuat(sbio::ig::BodyRotation bodyRot);

      /**
       * @brief Builds an Unreal transform from CIGI body position and rotation.
       * @param bodyPos CIGI body position.
       * @param bodyRot CIGI body rotation.
       * @return Unreal transform with unit scale.
       */
      FTransform BodyTransformToFTransform(sbio::math::BodyCoordinates bodyPos, sbio::ig::BodyRotation bodyRot);

      /**
       * @brief Converts an Unreal vector to CIGI reference-plane coordinates.
       * @param fv Unreal vector.
       * @return CIGI reference-plane coordinates.
       */
      sbio::math::ReferencePlaneCoordinates FVectorToReferencePlaneCoordinates(const FVector& fv);

      /**
       * @brief Converts CIGI reference-plane coordinates to an Unreal vector.
       * @param referencePlaneCoords Reference-plane coordinates to convert.
       * @return Converted Unreal vector.
       */
      FVector ReferencePlaneCoordinatesToFVector(const sbio::math::ReferencePlaneCoordinates& referencePlaneCoords);

      /**
       * @brief Converts an Unreal FVector to CIGI CigiBodyCoordinates.
       * @param fv Unreal FVector.
       * @return CIGI CigiBodyCoordinates struct.
       */
      sbio::cigi::CigiBodyCoordinates FVectorToCigiBodyCoordinates(FVector fv);

      /**
       * @brief Converts CIGI CigiBodyCoordinates to an Unreal FVector.
       * @param bodyCoordinates CIGI CigiBodyCoordinates struct.
       * @return Unreal FVector.
       */
      FVector CigiBodyCoordinatesToFVector(sbio::cigi::CigiBodyCoordinates bodyCoordinates);

      // --- Other Utilities ---

      /**
       * @brief Safely dereferences a pointer-to-pointer, returns null if the pointer is null.
       * @tparam T Pointer type.
       * @param ptr Pointer to pointer.
       * @return Dereferenced pointer or null.
       */
      template <typename T>
      T* Deref(T** ptr)
      {
        if (ptr == nullptr)
        {
          return nullptr;
        }
        return *ptr;
      }

      /**
       * @brief Safely gets the name of an object, returns "NULL" if the object is null.
       * @param obj UObject pointer.
       * @return Object name or "NULL".
       */
      FString ObjName(UObject* obj);

      /**
       * @brief Converts a transform to a string with configurable precision.
       * @param transform Transform to convert.
       * @param percision Number of decimal places.
       * @return String representation.
       */
      FString TransformString(FTransform transform, uint8 percision = 1);

      /**
       * @brief Converts a vector to a string with configurable precision.
       * @param vector Vector to convert.
       * @param precision Number of decimal places.
       * @return String representation.
       */
      FString VectorString(FVector vector, uint8 precision = 1);

      /**
       * @brief Draws a debug line between two points if debug drawing is enabled.
       * @param InWorld World context.
       * @param start Start point.
       * @param end End point.
       * @param color Line color.
       * @param duration Duration to display.
       */
      void DebugLine(const UWorld* InWorld, FVector start, FVector end, FColor color, float duration = 20.0);

      /**
       * @brief Draws a debug sphere at a point if debug drawing is enabled.
       * @param InWorld World context.
       * @param point Center point.
       * @param radius Sphere radius.
       * @param color Sphere color.
       * @param duration Duration to display.
       */
      void DebugSphere(const UWorld* InWorld, FVector point, double radius, FColor color, float duration = 20.0);

      /**
       * @brief Returns the parent folder of a given filepath.
       * @param inPath Input filepath.
       * @return Parent folder path.
       */
      FString GetParentFolder(FString inPath);

      /**
       * @brief Returns true if the string is a valid IPv4 address.
       * @param ipAddress Address string to validate.
       * @return True if valid IPv4, otherwise false.
       */
      bool IsValidIPv4Address(const FString& ipAddress);

      /**
       * @brief Finds all Blueprint assets that derive from the specified base class.
       * @param Base Base class.
       * @return Array of UClass pointers.
       */
      TArray<UClass*> FindBlueprintAssets(UClass* Base);

      /**
       * @brief Finds all UFont assets in the project.
       * @return Array of UFont pointers.
       */
      TArray<UFont*> FindAllFontAssets();
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026