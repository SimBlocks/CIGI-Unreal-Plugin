//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiSystemEventHandler.h"

#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiComponentDispatcher.h"
#include "CigiController.h"
#include "CoreMinimal.h"

using namespace sbio;
using namespace sbio::unrealcigi;

CUnrealCigiSystemEventHandler::CUnrealCigiSystemEventHandler(CUnrealCigiEventHandler& eventHandler) : EventHandler(eventHandler)
{
}

void CUnrealCigiSystemEventHandler::OnSetEventComponentStateMessage(const sbio::ig::system::SSetEventComponentStateMessage& data)
{
  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::EVENT, data.ComponentID.Value(), data.ComponentState, data.EventID.Value(), data.ComponentData);
}

void CUnrealCigiSystemEventHandler::OnSetSystemComponentStateMessage(const sbio::ig::system::SSetSystemComponentStateMessage& data)
{
  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::SYSTEM, data.ComponentID.Value(), data.ComponentState, data.SystemID.Value(), data.ComponentData);
}

void CUnrealCigiSystemEventHandler::OnSetHostConnectedMessage(const sbio::ig::network::SHostConnectedMessage& data)
{
  UE_LOG(LogCigiEventHandler, Log, TEXT("CIGI Host connected: %s"), UTF8_TO_TCHAR(data.sHostIP.c_str()));
}

void CUnrealCigiSystemEventHandler::OnSetHostDisconnectedMessage(const sbio::ig::network::SHostDisconnectedMessage& data)
{
  UE_LOG(LogCigiEventHandler, Warning, TEXT("CIGI Host disconnected: %s"), UTF8_TO_TCHAR(data.sHostIP.c_str()));
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026