//Copyright SimBlocks LLC 2016-2026

using System.IO;
using UnrealBuildTool;
using System;

public class unrealcigi : ModuleRules
{
  // Constructor
  public unrealcigi(ReadOnlyTargetRules Target) : base(Target)
  {
    // allow dynamic_cast
    bUseRTTI = true;

    // fix unwinding error when building an unreal package
    bEnableExceptions = true;

    // faster compile times?
    MinFilesUsingPrecompiledHeaderOverride = 1;
    bUseUnity = false;

    // default ue config
    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject" });

    // No need for any CIGI dependencies here. All CIGI code is in Plugins/UnrealCigi_Plugin.
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026