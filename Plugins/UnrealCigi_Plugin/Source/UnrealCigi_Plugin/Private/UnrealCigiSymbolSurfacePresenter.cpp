//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiSymbolSurfacePresenter.h"
#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiSymbolManager.h"
#include "UnrealCigiEntityManager.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiViewManager.h"
#include "Kismet/GameplayStatics.h"
#include "CigiEntity.h"
#include "CigiWidget.h"
#include "Blueprint/UserWidget.h"

using namespace sbio::symbol;

namespace sbio
{
  namespace unrealcigi
  {
    CUnrealCigiSymbolSurfacePresenter::CUnrealCigiSymbolSurfacePresenter(CUnrealCigiEventHandler& eventHandler) : EventHandler(eventHandler)
    {
    }

    void CUnrealCigiSymbolSurfacePresenter::UpdateSurfaceWidget(SymbolSurfaceID surfaceID, int32 newAttachID)
    {
      // Check if the Unreal symbol manager is available
      if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager == nullptr)
      {
        return;
      }

      FUnrealSymbolSurface* surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSurface(surfaceID);
      
      // If the surface is not found or its type is UNKNOWN, return early
      if (surface == nullptr || surface->Type == SymbolSurfaceType::UNKNOWN)
      {
        return;
      }

      // If the widget is not valid, create a new widget for this surface
      if (!IsValid(surface->Widget))
      {
        surface->Widget = CreateWidget<UCigiWidget>(EventHandler.GetWorld(), UCigiWidget::StaticClass());

        if (!IsValid(surface->Widget))
        {
          return;
        }

        surface->Widget->SetSurfaceID(surfaceID);
        surface->WidgetType = SymbolSurfaceType::UNKNOWN;
      }

      // Reattach the widget when either its presentation type or target changes.
      if (surface->Type != surface->WidgetType || surface->AttachID != newAttachID)
      {
        RemoveSurfaceWidget(surfaceID);
        if (surface->Type == SymbolSurfaceType::VIEW)
        {
          const int32 playerControllerIndex = FUnrealCigi_PluginModule::globals.pUnrealViewManager->PlayerControllerIndex(newAttachID);
          APlayerController* playerController = nullptr;
          if (playerControllerIndex >= 0)
          {
            playerController = UGameplayStatics::GetPlayerController(EventHandler.GetWorld(), playerControllerIndex);
          }
          if (!IsValid(playerController))
          {
            surface->AttachID = -1;
            return;
          }
          surface->Widget->SetOwningPlayer(playerController);
          surface->Widget->AddToPlayerScreen();
        }
        surface->AttachID = newAttachID;
        surface->WidgetType = surface->Type;
      }

      // If the widget is attached to an entity, update the entity's widget components
      if (surface->WidgetType == SymbolSurfaceType::BILLBOARD || surface->WidgetType == SymbolSurfaceType::WORLD)
      {
        ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(sbio::EntityID(surface->AttachID));
        if (IsValid(entity))
        {
          entity->UpdateWidgetComponent(surfaceID);
        }
      }
    }

    void CUnrealCigiSymbolSurfacePresenter::RemoveSurfaceWidget(SymbolSurfaceID surfaceID)
    {
      // Check if the Unreal symbol manager is available
      if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager == nullptr)
      {
        return;
      }

      FUnrealSymbolSurface* surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSurface(surfaceID);

      // If the surface is not found or the widget is invalid, return early
      if (surface == nullptr || !IsValid(surface->Widget))
      {
        return;
      }

      // Remove the widget from its parent (the viewport or entity)
      surface->Widget->RemoveFromParent();

      // If the widget is attached to an entity, remove it from the entity's widget components
      if (surface->WidgetType == SymbolSurfaceType::BILLBOARD || surface->WidgetType == SymbolSurfaceType::WORLD)
      {
        ACigiEntity* entity = FUnrealCigi_PluginModule::globals.pUnrealEntityManager->Find(sbio::EntityID(surface->AttachID));
        if (IsValid(entity))
        {
          entity->RemoveWidgetComponent(surfaceID);
        }
      }

      // Reset the widget and its type to UNKNOWN
      surface->WidgetType = SymbolSurfaceType::UNKNOWN;
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026