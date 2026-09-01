//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiSensorEventHandler.h"

#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiComponentDispatcher.h"
#include "CigiController.h"
#include "CoreMinimal.h"

using namespace sbio;
using namespace sbio::unrealcigi;

CUnrealCigiSensorEventHandler::CUnrealCigiSensorEventHandler(CUnrealCigiEventHandler& eventHandler) : EventHandler(eventHandler)
{
}

void CUnrealCigiSensorEventHandler::OnUpdateSensorMessage(const sbio::ig::sensor::SUpdateSensorMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnUpdateSensorMessage: Not currently supported"));
}

void CUnrealCigiSensorEventHandler::OnUpdateSensorComponentMessage(const sbio::ig::sensor::SUpdateSensorComponentMessage& data)
{
  FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::SENSOR, data.ComponentID.Value(), data.ComponentState, data.SensorID.Value(), data.ComponentData);
}

void CUnrealCigiSensorEventHandler::OnCreateMotionTrackerViewMessage(const sbio::ig::sensor::SCreateMotionTrackerViewMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateMotionTrackerViewMessage: Motion trackers are not currently supported"));
}

void CUnrealCigiSensorEventHandler::OnCreateMotionTrackerViewGroupMessage(const sbio::ig::sensor::SCreateMotionTrackerViewGroupMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnCreateMotionTrackerViewGroupMessage: Motion trackers are not currently supported"));
}

void CUnrealCigiSensorEventHandler::OnSetMotionTrackerMessage(const sbio::ig::sensor::SSetMotionTrackerMessage& data)
{
  UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetMotionTrackerMessage: Motion trackers are not currently supported"));
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026