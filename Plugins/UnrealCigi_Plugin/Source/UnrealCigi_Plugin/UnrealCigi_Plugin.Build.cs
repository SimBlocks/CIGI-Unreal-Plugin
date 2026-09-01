//Copyright SimBlocks LLC 2016-2026

using System.IO;
using UnrealBuildTool;
using System;

/// <summary>
/// Build rules for the SimBlocks UnrealCigi plugin module.
///
/// This class configures module dependencies, library paths, include paths for the plugin.
///
/// Usage:
/// - Add libraries and include paths for SimulationSDK and third-party dependencies.
/// </summary>
public class UnrealCigi_Plugin : ModuleRules
{
  /// <summary>
  /// Constructor. Sets up module rules, dependencies, libraries, and include paths.
  /// </summary>
  /// <param name="Target">Read-only target rules for the build.</param>
  public UnrealCigi_Plugin(ReadOnlyTargetRules Target) : base(Target)
  {
    // allow dynamic_cast
    bUseRTTI = true;

    // fix unwinding error when building an unreal package
    bEnableExceptions = true;

    // faster compile times?
    MinFilesUsingPrecompiledHeaderOverride = 1;
    bUseUnity = false;

    // Default Unreal modules plus UnrealCIGI modules
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    CppStandard = CppStandardVersion.Default;
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "Json", "Landscape", "UMG", "CelestialVault", "DaySequence", "GeoReferencing" });
    PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

    string sThirdParty = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "..", "..", "..", "thirdparty"));
    string sSimulationSDKPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "..", "..", "..", "simulationsdk"));

    // Edit the path to point to simulationsdk\Code instead
    string sSimSDKCode = Path.Combine(sSimulationSDKPath, "Code");

    // Determine if the current build configuration is Debug or DebugGame
    bool runtimeDebug = Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame;
    string sRuntimeConfiguration = runtimeDebug ? "Debug" : "Release";

    // The Debug versions of some .lib files have "d" or "_d" added to their names
    string sRuntimeSuffix = runtimeDebug ? "d" : "";
    string sGeographicSuffix = runtimeDebug ? "_d" : "";

    // Add a preprocessor definition to indicate whether the runtime is in debug mode
    PrivateDefinitions.Add(runtimeDebug ? "SBIO_RUNTIME_DEBUG=1" : "SBIO_RUNTIME_DEBUG=0");
    PrivateDefinitions.Add("POCO_NEW_STATE_ON_MOVE=1");

    // load the libraries
    AddLibraries(
      // simulationsdk
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioGlobalHeaders.lib"),
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioCigiLib.lib"),
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioIGCigiLib.lib"),
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioViewLib.lib"),
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioUtilitiesLib" + sRuntimeSuffix + ".lib"),
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioEngineLib.lib"),
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioEntityLib.lib"),
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioMathLib" + sRuntimeSuffix + ".lib"),
      Path.Combine(sSimSDKCode, "Bin", "vc143", "x64", "vs2022_" + sRuntimeConfiguration, "sbioSymbolLib.lib"),

      // tinyxml
      Path.Combine(sThirdParty, "build", "tinyxml2", "vs2022-x64", sRuntimeConfiguration, "tinyxml2.lib"),
      // geographic lib
      Path.Combine(sThirdParty, "build", "geographiclib", "vs2022-x64", "lib", sRuntimeConfiguration, "GeographicLib" + sGeographicSuffix + "-i.lib"),
      // poco-1.14.2
      Path.Combine(sThirdParty, "build", "poco", "vs2022-x64", "lib", "PocoFoundation" + sRuntimeSuffix + ".lib"),
      Path.Combine(sThirdParty, "build", "poco", "vs2022-x64", "lib", "PocoJSON" + sRuntimeSuffix + ".lib"),
      Path.Combine(sThirdParty, "build", "poco", "vs2022-x64", "lib", "PocoNet" + sRuntimeSuffix + ".lib")
      );

    // Include SimulationSDK headers.
    PublicIncludePaths.Add(Path.Combine(sSimSDKCode, "Libraries"));
    PublicIncludePaths.Add(Path.Combine(sSimSDKCode, "Libraries", "SymbolLib"));

    // Include Third-party headers privately.
    AddPrivateIncludePaths(
      // poco-1.14.2
      Path.Combine(sThirdParty, "poco", "Net", "include"),
      Path.Combine(sThirdParty, "poco", "Foundation", "include"),
      Path.Combine(sThirdParty, "poco", "Util", "include"),
      Path.Combine(sThirdParty, "poco", "XML", "include"),
      Path.Combine(sThirdParty, "poco", "JSON", "include"),

      // misc
      sThirdParty,
      Path.Combine(sThirdParty, "boost"),
      Path.Combine(sThirdParty, "eigen"),
      Path.Combine(sThirdParty, "geographiclib", "include"),
      Path.Combine(sThirdParty, "tinyxml2"),

      ""
      );
  }

  /// <summary>
  /// Adds native libraries required by the module.
  /// Each input path should be an absolute path.
  /// </summary>
  /// <param name="libPaths">Absolute paths to library files.</param>
  private void AddLibraries(params string[] libPaths)
  {
    foreach (string libPath in libPaths)
    {
      if (libPath == null || libPath.Length <= 0)
        continue;
      PublicAdditionalLibraries.Add(libPath);
    }
  }

  /// <summary>
  /// Adds include paths to the module's PrivateIncludePaths list.
  /// Each input path should be an absolute path.
  /// </summary>
  /// <param name="includePaths">Absolute paths to include directories.</param>
  private void AddPrivateIncludePaths(params string[] includePaths)
  {
    foreach (string includePath in includePaths)
    {
      if (includePath == null || includePath.Length <= 0)
        continue;
      PrivateIncludePaths.Add(includePath);
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026