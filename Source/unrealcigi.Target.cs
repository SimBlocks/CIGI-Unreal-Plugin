//Copyright SimBlocks LLC 2016-2026
//Portions of this file Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class unrealcigiTarget : TargetRules
{
  public unrealcigiTarget(TargetInfo Target) : base(Target)
  {
    Type = TargetType.Game;
    DefaultBuildSettings = BuildSettingsVersion.Latest;
    ExtraModuleNames.Add("unrealcigi");
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026