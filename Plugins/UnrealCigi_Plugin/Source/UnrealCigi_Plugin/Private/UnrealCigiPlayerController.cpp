//Copyright SimBlocks LLC 2016-2026

#include "unrealcigiPlayerController.h"
#include "CigiHUD.h"

using namespace sbio;

DEFINE_LOG_CATEGORY(LogCigiPlayerController)

AunrealcigiPlayerController::AunrealcigiPlayerController()
{
  bShowMouseCursor = true;
  bEnableClickEvents = true;
  bEnableTouchEvents = true;
  DefaultMouseCursor = EMouseCursor::Crosshairs;

  ViewID = sbio::ViewID(0);
  Widgets.Empty();
}

void AunrealcigiPlayerController::SetViewID(sbio::ViewID viewID)
{
  ViewID = viewID;

  // Update the HUD with the new ViewID
  ACigiHUD* hud = Cast<ACigiHUD>(GetHUD());

  // Check if the HUD is valid before attempting to set the ViewID
  if (!IsValid(hud))
  {
    return;
  }

  // Set the ViewID in the HUD as well
  hud->ViewID = viewID;
}

sbio::ViewID AunrealcigiPlayerController::GetViewID() const
{
  return ViewID;
}

void AunrealcigiPlayerController::AddWidget(int symbolSurfaceID)
{
  // Create a new CigiWidget instance
  UCigiWidget* widget = CreateWidget<UCigiWidget>(this, UCigiWidget::StaticClass());

  // Check if the widget was created successfully
  if (!IsValid(widget))
  {
    UE_LOG(LogCigiPlayerController, Warning, TEXT("AddWidget: Failed to create widget!"));
    return;
  }

  // Set the symbol surface ID and add the widget to the player's screen
  widget->SetSurfaceID(sbio::symbol::SymbolSurfaceID(symbolSurfaceID));
  widget->SetOwningPlayer(this);
  widget->AddToPlayerScreen();

  // Store the widget in the map for later removal
  Widgets.Add(symbolSurfaceID, widget);
}

void AunrealcigiPlayerController::RemoveAllWidgets()
{
  // Iterate through the map of widgets and remove each one from its parent (the viewport)
  for (auto& pair : Widgets)
  {
    // Get the widget from the map
    UCigiWidget* widget = pair.Value;

    // Check if the widget is valid before attempting to remove it
    if (!IsValid(widget))
    {
      continue;
    }

    // Remove the widget from its parent (the viewport)
    widget->RemoveFromParent();
  }

  // Clear the map of widgets after removing them from the parent
  Widgets.Empty();
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026