//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiEnvironmentManager.h"

namespace sbio
{
  namespace unrealcigi
  {
    void CUnrealCigiEnvironmentManager::Reset()
    {
      WeatherLayers.Empty();
      AtmosphereInfo = FWeatherLayer();
      AtmosphereEnabled = false;
    }

    void CUnrealCigiEnvironmentManager::SetAtmosphereEnabled(bool enabled)
    {
      AtmosphereEnabled = enabled;
    }

    void CUnrealCigiEnvironmentManager::SetAtmosphere(const sbio::ig::atmosphere::SSetAtmosphereMessage& data)
    {
      AtmosphereInfo = FWeatherLayer();
      AtmosphereInfo.AirTemperature = data.AirTemperature;
      AtmosphereInfo.BarometricPressure = data.BarometricPressure;
      AtmosphereInfo.HorizontalWindSpeed = data.HorizontalWindSpeed;
      AtmosphereInfo.Humidity = data.Humidity;
      AtmosphereInfo.VerticalWindSpeed = data.VerticalWindSpeed;
      AtmosphereInfo.VisibilityRange = data.VisibilityRange;
      AtmosphereInfo.WindDirection = data.WindDirection;
    }

    void CUnrealCigiEnvironmentManager::SetWeather(const sbio::ig::atmosphere::SSetWeatherMessage& data)
    {
      // Update or add the weather layer with the given LayerID
      FWeatherLayer& layer = WeatherLayers.FindOrAdd(data.LayerID);
      layer.WeatherEnabled = data.WeatherEnabled;
      layer.LayerID = data.LayerID;
      layer.Humidity = data.Humidity;
      layer.AirTemperature = data.AirTemperature.Value();
      layer.VisibilityRange = data.VisibilityRange;
      layer.HorizontalWindSpeed = data.HorizontalWindSpeed;
      layer.VerticalWindSpeed = data.VerticalWindSpeed;
      layer.WindDirection = data.WindDirection;
      layer.BarometricPressure = data.BarometricPressure;
      layer.AerosolConcentration = data.AerosolConcentration;
      layer.BottomScudEnabled = data.BottomScudEnabled;
      layer.TopScudEnabled = data.TopScudEnabled;
      layer.TopScudFrequency = data.TopScudFrequency;
      layer.BottomScudFrequency = data.BottomScudFrequency;
      layer.RandomWindsEnabled = data.RandomWindsEnabled;
      layer.RandomLightningEnabled = data.RandomLightningEnabled;
      layer.CloudType = data.CloudType;
      layer.Severity = static_cast<EUnrealCigiWeatherSeverity>(data.Severity.Value());
      layer.Coverage = data.Coverage;
      layer.BaseElevation = data.BaseElevation;
      layer.VerticalThickness = data.VerticalThickness;
      layer.TopTransitionBandThickness = data.TopTransitionBandThickness;
      layer.BottomTransitionBandThickness = data.BottomTransitionBandThickness;
    }

    FWeatherLayer CUnrealCigiEnvironmentManager::GetWeatherLayer(int32 layerID) const
    {
      // Find the weather layer with the given layerID
      const FWeatherLayer* layer = WeatherLayers.Find(layerID);

      // If the layer is not found, return a default FWeatherLayer with LayerID set to -1
      if (layer == nullptr)
      {
        FWeatherLayer result;
        result.LayerID = -1;
        return result;
      }

      return *layer;
    }

    TMap<int, FWeatherLayer> CUnrealCigiEnvironmentManager::GetWeatherLayers() const
    {
      return WeatherLayers;
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026