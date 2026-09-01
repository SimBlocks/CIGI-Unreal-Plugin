//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiEnvironmentEventHandler.h"
#include "UnrealCigiEventHandler.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiEnvironmentManager.h"
#include "CigiController.h"

namespace sbio
{
  namespace unrealcigi
  {
    void CUnrealCigiEnvironmentEventHandler::OnSetAtmosphereEnabledMessage(const sbio::ig::atmosphere::SSetAtmosphereEnabledMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pEnvironmentManager->SetAtmosphereEnabled(data.Enabled);
      FString enableString = data.Enabled ? FString("True") : FString("False");
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("Received OnSetAtmosphereEnabledMessage. Enabled = %s"), *enableString);
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetAtmosphereMessage(const sbio::ig::atmosphere::SSetAtmosphereMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pEnvironmentManager->SetAtmosphere(data);
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("Received OnSetAtmosphereMessage. Data stored."));
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetWeatherMessage(const sbio::ig::atmosphere::SSetWeatherMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pEnvironmentManager->SetWeather(data);
      UE_LOG(LogCigiEventHandler, CIGI_LOG, TEXT("Received OnSetWeatherMessage. Data stored."));
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetRegionalLayeredWeatherComponentStateMessage(const sbio::ig::atmosphere::SSetRegionalLayeredWeatherComponentStateMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::REGIONAL_WEATHER, data.ComponentID.Value(), data.ComponentState, data.LayeredWeatherID.Value(), data.ComponentData);
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetGlobalLayeredWeatherComponentStateMessage(const sbio::ig::atmosphere::SSetGlobalLayeredWeatherComponentStateMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::GLOBAL_WEATHER, data.ComponentID.Value(), data.ComponentState, data.LayeredWeatherID.Value(), data.ComponentData);
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetAtmosphereComponentStateMessage(const sbio::ig::atmosphere::SSetAtmosphereComponentStateMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::ATMOSPHERE, data.ComponentID.Value(), data.ComponentState, data.AtmosphereID.Value(), data.ComponentData);
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetMaritimeSurfaceConditionsMessage(const sbio::ig::ocean::SSetMaritimeSurfaceConditionsMessage& data)
    {
      UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetMaritimeSurfaceConditionsMessage: Maritime conditions are not currently supported."));
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetRegionMaritimeComponentStateMessage(const sbio::ig::ocean::SSetRegionMaritimeComponentStateMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::REGIONAL_MARITIME, data.ComponentID.Value(), data.ComponentState, data.RegionID.Value(), data.ComponentData);
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetGlobalMaritimeComponentStateMessage(const sbio::ig::ocean::SSetGlobalMaritimeComponentStateMessage& data)
    {
      FUnrealCigi_PluginModule::globals.pComponentDispatcher->Process(ComponentClass::GLOBAL_MARITIME, data.ComponentID.Value(), data.ComponentState, data.MaritimeID.Value(), data.ComponentData);
    }

    void CUnrealCigiEnvironmentEventHandler::OnSetEarthReferenceModelMessage(const sbio::ig::earth::SSetEarthReferenceModelMessage& data)
    {
      UE_LOG(LogCigiEventHandler, CIGI_WARNING, TEXT("OnSetEarthReferenceModelMessage: Not currently supported."));
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026