//Copyright SimBlocks LLC 2016-2026

#include "CigiController.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiEventHandler.h"

using namespace sbio::unrealcigi;

// ----- Component Message

FComponentMessage::FComponentMessage(int componentId, int componentState, int instanceId, FComponentData data)
{
  ComponentId = componentId;
  ComponentState = componentState;
  InstanceId = instanceId;
  Data = data;
}

FComponentMessage::FComponentMessage()
{
  ComponentId = 0;
  ComponentState = 0;
  InstanceId = 0;
  Data = FComponentData();
}

FString FComponentMessage::ToString()
{
  FString dataText = Data.ToString();
  FString msgText = FString::Printf(TEXT("ComponentMessage(id=%d,state=%d,instance=%d,data=%s)"), ComponentId, ComponentState, InstanceId, *dataText);
  return msgText;
}

// ----- Cigi Controller -----

ACigiController::ACigiController()
{
  // Disable Event Tick because CigiController only responds to messages from EventHandler
  PrimaryActorTick.bCanEverTick = false;
}

void ACigiController::BeginPlay()
{
  Super::BeginPlay();

  // Get the world context for this actor. If the world is null, log a warning and return.
  UWorld* world = GetWorld();
  if (world == nullptr)
  {
    UE_LOG(LogTemp, Warning, TEXT("CigiController: Could not find World!"));
    return;
  }

  // Register with the event handler whenever the actor has a valid world.
  // The handler is the source of CIGI packets and is also available on clients.
  CUnrealCigiEventHandler* eh = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (eh == nullptr)
  {
    UE_LOG(LogTemp, Warning, TEXT("CigiController: Could not find EventHandler!"));
    return;
  }

  // Register the CigiController so the CIGI event handler will call this controller's events.
  std::vector<TWeakObjectPtr<ACigiController>>& controllers = FUnrealCigi_PluginModule::globals.pComponentDispatcher->Controllers;
  for (int32 controllerIndex = static_cast<int32>(controllers.size()) - 1; controllerIndex >= 0; --controllerIndex)
  {
    if (controllers[controllerIndex].Get() == this)
    {
      controllers.erase(controllers.begin() + controllerIndex);
    }
  }

  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Controllers.push_back(TWeakObjectPtr<ACigiController>(this));
  UE_LOG(LogTemp, Log, TEXT("CigiController \"%s\": Registered with the EventHandler"), *GetName());
}

void ACigiController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  if (CUnrealCigiEventHandler* eh = FUnrealCigi_PluginModule::globals.pEventHandler.get())
  {
    // Unregister the CigiController from the event handler when this actor is destroyed or removed from the world.
    std::vector<TWeakObjectPtr<ACigiController>>& controllers = FUnrealCigi_PluginModule::globals.pComponentDispatcher->Controllers;
    for (int32 controllerIndex = static_cast<int32>(controllers.size()) - 1; controllerIndex >= 0; --controllerIndex)
    {
      if (controllers[controllerIndex].Get() == this)
      {
        controllers.erase(controllers.begin() + controllerIndex);
      }
    }
  }

  Super::EndPlay(EndPlayReason);
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026