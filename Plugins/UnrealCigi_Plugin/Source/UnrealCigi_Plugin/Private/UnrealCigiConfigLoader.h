//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

namespace sbio
{
  namespace unrealcigi
  {
    class CUnrealCigiViewEventHandler;

    /**
     * @class CUnrealCigiConfigLoader
     * @brief Loads UnrealCigi configuration data from the configured source.
     */
    class CUnrealCigiConfigLoader
    {
    public:
      /**
       * @brief Finds and parses the UnrealCigi JSON configuration file.
       *
       * Searches for UnrealCigi.config.json from the project directory upward
       * through its parent directories.
       *
       * @param filePath Receives the absolute path of the configuration file being examined.
       * @return The parsed JSON object, or nullptr if no valid configuration file is found.
       */
      static TSharedPtr<FJsonObject> LoadJsonConfig(FString& filePath);

      /**
       * @brief Loads UnrealCigi configuration and applies it to the view event handler.
       * @param viewEventHandler View event handler that receives the loaded configuration.
       */
      static void LoadConfig(CUnrealCigiViewEventHandler& viewEventHandler);

      /**
       * @brief Releases configuration objects retained in Unreal's root set.
       *
       * Call this before UObject shutdown so retained configuration references are
       * released while the referenced objects are still valid.
       */
      static void ReleaseRootedConfigObjects();

      /**
       * @brief Parses a three-component vector from a JSON array field.
       *
       * Missing, invalid, or nonnumeric components retain their default value of zero.
       *
       * @param object JSON object containing the vector field.
       * @param fieldName Name of the array field to parse.
       * @return Vector populated from up to the first three array elements.
       */
      static FVector ParseVector(const TSharedPtr<FJsonObject>& object, const FString& fieldName);

      /**
       * @brief Converts a project-relative asset path to an Unreal content path.
       *
       * Paths are prefixed with /Game/. The special value "none" is returned unchanged,
       * using a case-insensitive comparison.
       *
       * @param filepath Project-relative asset path or the special value "none".
       * @return The corresponding Unreal content path, or the unchanged special value.
       */
      static FString FullAssetPath(const FString& filepath);
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026