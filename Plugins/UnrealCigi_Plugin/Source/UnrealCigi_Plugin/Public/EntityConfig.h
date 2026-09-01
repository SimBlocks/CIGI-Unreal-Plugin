//Copyright SimBlocks LLC 2016-2026
/**
 * @file EntityConfig.h
 * @brief Defines configuration structures and classes for CIGI entity types in Unreal Engine.
 *
 * This header provides:
 * - FArticulatedPartConfig: Struct for configuring articulated parts of an entity.
 * - UEntityConfig: UObject class for storing entity configuration, supporting both Blueprint and JSON-based configs.
 *
 * Usage:
 * - Use UEntityConfig to store and initialize entity configuration data for spawning and setup.
 * - Use FArticulatedPartConfig to describe individual articulated parts for an entity type.
 */

#pragma once

#include "ModuleAPI.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EngineLib/ImageGeneratorMessages.h"
#include "CigiEntity.h"
#include "EntityConfig.generated.h"

/**
 * @struct FArticulatedPartConfig
 * @brief Configuration for an articulated part of an entity.
 * @details Stores name, asset filepath, origin, and transform for the part.
 */
USTRUCT()

struct FArticulatedPartConfig
{
  GENERATED_USTRUCT_BODY()

  /** Name of the articulated part. */
  UPROPERTY()
  FString name;

  /** Filepath to the asset for this part. */
  UPROPERTY()
  FString filepath;

  /** Origin position for the part. */
  UPROPERTY()
  FVector origin;

  /** Transform to apply to the part. */
  UPROPERTY()
  FTransform transform;

  /**
   * @brief Constructs an articulated part config with all fields.
   * @param partName Name of the part.
   * @param partFilepath Asset filepath.
   * @param partOrigin Origin position.
   * @param partTransform Transform to apply.
   */
  FArticulatedPartConfig(FString partName, FString partFilepath, FVector partOrigin, FTransform partTransform);

  /**
   * @brief Default constructor.
   */
  FArticulatedPartConfig();
};

/**
 * @class UEntityConfig
 * @brief Stores configuration info for a single entity type (Blueprint or JSON).
 * @details Supports both Blueprint asset reference and JSON-parsed configuration for entity setup.
 */
UCLASS()

class MODULE_API UEntityConfig : public UObject
{
  GENERATED_BODY()
public:
  /**
   * @brief Blueprint config: Reference to the Blueprint asset. Non-null for BP config, null for JSON config.
   */
  UPROPERTY()
  TSubclassOf<ACigiEntity> blueprint;

  /**
   * @brief JSON config: Display name of the entity.
   */
  UPROPERTY()
  FString name;

  /**
   * @brief JSON config: Filepath to the main visual asset.
   */
  UPROPERTY()
  FString filepath;

  /**
   * @brief JSON config: Mesh transform of the main visual asset.
   */
  UPROPERTY()
  FTransform transform;

  /**
   * @brief JSON config: Information for creating the entity's articulated parts (if any).
   */
  UPROPERTY()
  TMap<int32, struct FArticulatedPartConfig> articulatedParts;

  /**
   * @brief JSON config: Information for registering the entity's articulated bones (if any).
   */
  UPROPERTY()
  TMap<FString, int32> articulatedBones;

public:
  /**
   * @brief Initializes this config object as a JSON config.
   * Stores all entity data parsed from the JSON file.
   * @param _name Display name of the entity.
   * @param _filepath Filepath to the main visual asset.
   * @param _transform Mesh transform.
   * @param _articulatedParts Map of articulated part configs.
   * @param _articulatedBones Map of articulated bone names to IDs.
   */
  UFUNCTION()
  void InitJSON(FString _name, FString _filepath, FTransform _transform, TMap<int32, FArticulatedPartConfig> _articulatedParts, TMap<FString, int32> _articulatedBones);

  /**
   * @brief Initializes this config object as a Blueprint config.
   * Stores a pointer to the Blueprint asset.
   * @param _blueprint Reference to the Blueprint asset.
   */
  UFUNCTION()
  void InitBP(TSubclassOf<ACigiEntity> _blueprint);

  /**
   * @brief Default constructor. Initializes variables.
   */
  UEntityConfig();

  /**
   * @brief Outputs a single string that displays the key properties of this config.
   * @return String describing the config.
   */
  FString ToString() const;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026