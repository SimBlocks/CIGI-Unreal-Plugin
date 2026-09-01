//Copyright SimBlocks LLC 2016-2026

// First Header
#include "unrealcigiGameMode.h"

#ifdef GetObject
#undef GetObject
#endif

#include "CigiView.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiEventHandler.h"
#include "CigiHUD.h"
#include "unrealcigiPlayerController.h"


DEFINE_LOG_CATEGORY(LogCigiGameMode)

using namespace std;
using namespace sbio;
using namespace sbio::utils;
using namespace sbio::entity;
using namespace sbio::symbol;
using namespace sbio::engine;
using namespace sbio::view;
using namespace sbio::cigi;
using namespace sbio::cigi::ig;
using namespace sbio::ig;
using namespace sbio::unrealcigi;

AunrealcigiGameMode::AunrealcigiGameMode()
{
  // This game mode must tick so the simulationsdk image generator code runs properly
  PrimaryActorTick.bCanEverTick = true;

  // We do not need to tick when the game is paused
  PrimaryActorTick.bTickEvenWhenPaused = false;

  // Tick before everything else, so other Tick events will have the most recent CIGI packet info
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  // Tick as often as possible, so we can send and receive CIGI packets quickly (low ping)
  PrimaryActorTick.TickInterval = 0;

  // Default pawn is the CigiView class, which is controlled by the CIGI View Control packets
  PlayerControllerClass = AunrealcigiPlayerController::StaticClass();
  DefaultPawnClass = ACigiView::StaticClass();

  // Default HUD is the CigiHUD, which supports the CIGI widgets and slate code for CIGI symbol surfaces
  HUDClass = ACigiHUD::StaticClass();
}

void AunrealcigiGameMode::BeginPlay()
{
  Super::BeginPlay();
  // Startup the simulationsdk image generator
  FUnrealCigi_PluginModule::StartIG();

  // Start playing the simulationsdk image generator, which will start sending and receiving CIGI packets
  if (FUnrealCigi_PluginModule::globals.pImageGenerator)
  {
    FUnrealCigi_PluginModule::globals.pImageGenerator->StartPlaying();
  }

  UE_LOG(LogCigiGameMode, Log, TEXT("Finished BeginPlay setup"));
}

void AunrealcigiGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // Shut down the simulationsdk image generator
  if (FUnrealCigi_PluginModule::globals.pImageGenerator)
  {
    FUnrealCigi_PluginModule::globals.pImageGenerator->StopPlaying();
  }

  FUnrealCigi_PluginModule::StopIG();
  Super::EndPlay(EndPlayReason);
}

void AunrealcigiGameMode::Tick(float DeltaSeconds)
{
  // Initialize the event handler AFTER BeginPlay()
  if (!eventHandlerInitialized)
  {
    // This triggers once only, on the first Tick
    if (CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get())
    {
      eventHandler->Initialize(GetWorld());
      eventHandlerInitialized = true;
    }
  }

  // Update the simulation sdk image generator. This handles sending/receiving CIGI packets
  if (FUnrealCigi_PluginModule::globals.pImageGenerator)
  {
    FUnrealCigi_PluginModule::globals.pImageGenerator->Update(DeltaSeconds);
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026