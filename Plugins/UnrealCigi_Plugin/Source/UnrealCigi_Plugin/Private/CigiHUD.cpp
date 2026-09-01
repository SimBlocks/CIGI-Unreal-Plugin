//Copyright SimBlocks LLC 2016-2026

#include "CigiHUD.h"
#include "Engine/Engine.h"
#include "CigiView.h"
#include "Camera/CameraComponent.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiEventHandler.h"
#include "UnrealCigiViewManager.h"
#include "CigiEntity.h"

using namespace sbio::unrealcigi;

ACigiHUD::ACigiHUD()
{
}

void ACigiHUD::DrawHUD()
{
}

FVector2D ACigiHUD::GetActiveViewportSize()
{
  FVector2D size = FVector2D(1, 1);

  // By default, the size of the HUD is the size of the game viewport
  if (GEngine && GEngine->GameViewport)
  {
    GEngine->GameViewport->GetViewportSize(size);
    UE_LOG(LogTemp, Log, TEXT("found viewport size %s"), *size.ToString());
  }

  // If a valid view does not exist for this ViewID, then return the default size
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (eventHandler == nullptr)
  {
    return size;
  }

  // Try to find the CigiView associated with this HUD's ViewID
  ACigiView* view = nullptr;
  if (FUnrealCigi_PluginModule::globals.pUnrealViewManager != nullptr)
  {
    view = FUnrealCigi_PluginModule::globals.pUnrealViewManager->Find(ViewID.Value());
  }
  if (!IsValid(view))
  {
    return size;
  }
  // Try to get a reference to the CigiView pawn's camera component, if it exists
  UCameraComponent* cameraComp = view->CameraComp;
  if (!IsValid(cameraComp) || !cameraComp->bConstrainAspectRatio)
  {
    return size;
  }
  float ar = cameraComp->AspectRatio;

  // If a camera component was found, use its aspect ratio to adjust the game viewport size
  // This is the math that excludes the black bars on the screen (which are caused by the camera's aspect ratio)
  if (size.X / ar < size.Y)
  {
    size.Y = size.X / ar;
    UE_LOG(LogTemp, Warning, TEXT("adjusting Y for aspect ratio, new size is %s"), *size.ToString());
  }
  else
  {
    size.X = size.Y * ar;
    UE_LOG(LogTemp, Warning, TEXT("adjusting X for aspect ratio, new size is %s"), *size.ToString());
  }

  return size;
}

FVector2D ACigiHUD::PerToPix(float percentX, float percentY)
{
  // Convert percentages to pixels based on the active viewport size
  FVector2D result = GetActiveViewportSize();
  result = FVector2D(result.X * percentX, result.Y * percentY);
  UE_LOG(LogTemp, Warning, TEXT("percentage into pixels: (%.1f,%.1f) INTO %s"), percentX, percentY, *result.ToString());
  return result;
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026