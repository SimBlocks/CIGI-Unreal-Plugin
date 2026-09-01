//Copyright SimBlocks LLC 2016-2026
/**
 * @file UnrealCigi_Plugin.h
 * @brief Main public header for the SimBlocks UnrealCigi plugin module.
 *
 * This header provides:
 * - The FUnrealCigi_PluginModule class, the public module interface for UnrealCigi.
 * - Functions for starting and stopping the CIGI image generator.
 *
 * Usage:
 * - Include this file in modules that need to interact with the plugin lifecycle.
 * - Use FUnrealCigi_PluginModule to start and stop the image generator.
 * - SDK-dependent global resources are private implementation details.
 */

#pragma once

// Try to prevent windows headers from breaking Unreal
// (windows definition of min/max macros conflicts with Slate/SSpinBox)
#define NOMINMAX

// Unreal headers
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FJsonObject;

#ifdef GetObject
#undef GetObject
#endif

/**
 * @namespace sbio::unrealcigi
 * @brief SimBlocks UnrealCigi plugin namespace.
 */
namespace sbio
{
  struct SGlobals;

  namespace unrealcigi
  {
    struct SUnrealCigiGlobals;

    /**
     * @struct SUnrealCigiGlobals
     * @brief Global resource struct for UnrealCigi plugin.
     *
     * Holds shared pointers to entity, view, and symbol managers, event handler, event messenger, image generator, and exported function dispatcher.
     * Used to share resources between plugin subsystems.
     */
    /**
     * @class FUnrealCigi_PluginModule
     * @brief Main module class for the UnrealCigi plugin.
     *
     * Implements Unreal's IModuleInterface for plugin lifecycle management.
     * Provides static functions for starting/stopping the image generator and initializing plugin globals.
     */
    class FUnrealCigi_PluginModule : public IModuleInterface
    {
    public:
      /**
       * @brief Called by Unreal to start the plugin module.
       */
      virtual void StartupModule();

      /**
       * @brief Called by Unreal to shut down the plugin module.
       */
      virtual void ShutdownModule();

      /**
       * @brief Global resource struct for plugin subsystems.
       */
      static SUnrealCigiGlobals globals;

      /**
       * @brief Starts the image generator and connects to the host.
       */
      static void StartIG();
      /**
       * @brief Stops the image generator and disconnects from the host.
       */
      static void StopIG();

    private:
      /**
       * @brief Tracks whether globals have been initialized.
       */
      static bool globalsInitialized;

      /**
       * @brief Initializes global resources on first IG startup.
       */
      static void InitializeGlobals();

      /**
       * @brief Releases rooted UObjects before Unreal begins UObject shutdown.
       */
      void HandleEnginePreExit();

      /**
       * @brief Handle for the engine pre-exit delegate.
       */
      FDelegateHandle EnginePreExitHandle;
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026