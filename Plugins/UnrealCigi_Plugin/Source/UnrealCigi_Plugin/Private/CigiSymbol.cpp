//Copyright SimBlocks LLC 2016-2026

#include "CigiSymbol.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiEventHandler.h"
#include "unrealcigiUtil.h"
#include "CigiView.h"
#include "CigiEntity.h"

using namespace sbio;
using namespace sbio::symbol;
using namespace sbio::unrealcigi;

FUnrealSymbolSurface::FUnrealSymbolSurface(SymbolSurfaceID surfaceID)
{
  // When constructing a new symbol surface, set all variables to their default values
  SurfaceID = surfaceID;
  Type = SymbolSurfaceType::UNKNOWN;
  MinUV = FVector2D(0, 0);
  MaxUV = FVector2D(0, 0);
  Enabled = true;

  AttachID = -1;
  Offset = FVector(0, 0, 0);
  Size = FVector2D(0, 0);
  Rotation = FQuat::MakeFromEuler(FVector(0, 0, 0));

  Widget = nullptr;
}

FString FUnrealSymbolSurface::ToString() const
{
  // Take all the main info about this symbol surface and convert it into a single string
  return FString::Printf(TEXT("{type=%s,min=(%.1f,%.1f),max=[%.1f,%.1f],en=%d,attachID=%d,offset=(%.1f,%.1f,%.1f),size=(%.1f,%.1f),rot=(%.1f,%.1f,%.1f)}"),
                         Type == SymbolSurfaceType::VIEW ? TEXT("VIEW") : (Type == SymbolSurfaceType::BILLBOARD ? TEXT("BILLBOARD") : (Type == SymbolSurfaceType::WORLD ? TEXT("WORLD") : TEXT("UNKNOWN"))), MinUV.X, MinUV.Y, MaxUV.X, MaxUV.Y, Enabled, AttachID, Offset.X, Offset.Y, Offset.Z, Size.X,
                         Size.Y, Rotation.Euler().X, Rotation.Euler().Y, Rotation.Euler().Z);
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026