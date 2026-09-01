//Copyright SimBlocks LLC 2016-2026
/**
 * @file CigiBPLib.h
 * @brief Blueprint-accessible function library and types for SimBlocks UnrealCIGI plugin utilities.
 *
 * This header provides:
 * - EUnrealCigiWeatherSeverity: Enum for weather severity levels.
 * - FWeatherLayer: Struct describing a weather layer's properties.
 * - UCigiBPLib: Blueprint function library for querying entities, weather, and performing coordinate conversions.
 *
 * Usage:
 * - Use UCigiBPLib static functions in Blueprints or C++ to access entity, weather, and coordinate utilities.
 * - FWeatherLayer and EUnrealCigiWeatherSeverity are used for weather simulation and queries.
 */

#pragma once

#include "ModuleAPI.h"
#include "CoreMinimal.h"
#include "UnrealCigi_Plugin.h"
#include "CigiBPLib.generated.h"

/**
 * @brief Weather severity levels for CIGI simulation.
 */
UENUM(BlueprintType)
enum class EUnrealCigiWeatherSeverity : uint8
{
  NONE,///< No severity
  MARGINAL,///< Marginal severity
  SLIGHT,///< Slight severity
  ENHANCED,///< Enhanced severity
  MODERATE,///< Moderate severity
  HIGH,///< High severity
};

/**
 * @brief Represents a weather layer for CIGI simulation.
 */
USTRUCT(BlueprintType)

struct FWeatherLayer
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  bool WeatherEnabled = false;///< Is this weather layer enabled
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  int LayerID = 0;///< Unique layer identifier
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float Humidity = 0.0f;///< Humidity value
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float AirTemperature = 0.0f;///< Air temperature in Celsius
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float VisibilityRange = 0.0f;///< Visibility range in meters
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float HorizontalWindSpeed = 0.0f;///< Horizontal wind speed in m/s
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float VerticalWindSpeed = 0.0f;///< Vertical wind speed in m/s
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float WindDirection = 0.0f;///< Wind direction in degrees
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float BarometricPressure = 0.0f;///< Barometric pressure in hPa
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float AerosolConcentration = 0.0f;///< Aerosol concentration
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  bool BottomScudEnabled = false;///< Is bottom scud enabled
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  bool TopScudEnabled = false;///< Is top scud enabled
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float TopScudFrequency = 0.0f;///< Top scud frequency
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float BottomScudFrequency = 0.0f;///< Bottom scud frequency
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  bool RandomWindsEnabled = false;///< Is random wind enabled
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  bool RandomLightningEnabled = false;///< Is random lightning enabled
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  int CloudType = 0;///< Cloud type identifier
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  EUnrealCigiWeatherSeverity Severity = EUnrealCigiWeatherSeverity::NONE;///< Weather severity
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float Coverage = 0.0f;///< Cloud coverage
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float BaseElevation = 0.0f;///< Base elevation in meters
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float VerticalThickness = 0.0f;///< Vertical thickness in meters
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float TopTransitionBandThickness = 0.0f;///< Top transition band thickness
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CIGI|Weather")
  float BottomTransitionBandThickness = 0.0f;///< Bottom transition band thickness
};

/**
 * @brief Blueprint function library for CIGI plugin utilities.
 *
 * UCigiBPLib exposes static helper functions for querying entities, weather layers, and performing coordinate conversions.
 * All functions are accessible from Unreal Engine Blueprints and C++.
 *
 * - Entity lookup by CIGI ID
 * - Weather layer and atmosphere queries
 * - Wind speed and direction utilities
 * - Conversion functions for SISO IDs, component data/messages, and enums
 * - Geodetic/Unreal coordinate conversions, including Local Tangent Plane (LTP) support
 *
 * @copyright SimBlocks LLC 2016-2026
 */
UCLASS()

class MODULE_API UCigiBPLib : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:
  /**
   * @brief Finds an entity actor by CIGI entity identifier.
   * @param entityID CIGI entity identifier.
   * @return Matching entity actor, or nullptr when no entity is found.
   */
  UFUNCTION(BlueprintPure, Category = "Cigi|Utilities")
  static ACigiEntity* FindEntityFromID(int64 entityID);

  /**
   * @brief Returns a weather layer by identifier.
   * @param layerID Weather layer identifier.
   * @return The requested weather layer.
   */
  UFUNCTION(BlueprintPure, Category = "Cigi|Utilities")
  static FWeatherLayer WeatherLayerFromID(int layerID);

  /**
   * @brief Returns all available weather layers.
   * @return Map of weather layer identifiers to weather layers.
   */
  UFUNCTION(BlueprintPure, Category = "Cigi|Utilities")
  static TMap<int, FWeatherLayer> GetAllWeatherLayers();

  /**
   * @brief Returns the current atmosphere information.
   * @return Current atmosphere weather layer.
   */
  UFUNCTION(BlueprintPure, Category = "Cigi|Utilities")
  static FWeatherLayer GetAtmosphereInfo();

  /**
   * @brief Reports whether the atmosphere is enabled.
   * @return True when the atmosphere is enabled; otherwise false.
   */
  UFUNCTION(BlueprintPure, Category = "Cigi|Utilities")
  static bool IsAtmosphereEnabled();

  /** Returns the highest wind speed (in m/s) found in all of the weather layers.
   * If the atmosphere is enabled, this also considers the atmosphere wind speed. */
  UFUNCTION(BlueprintPure, Category = "Cigi|Utilities")
  static float GetWindSpeed();

  /** Returns an Unreal yaw (Z) rotation specifying the wind direction.
   * If the atmosphere is enabled, this returns the atmosphere's wind direction.
   * Otherwise, this returns the wind direction of the first enabled weather layer. */
  UFUNCTION(BlueprintPure, Category = "Cigi|Utilities")
  static float GetWindYaw();

  /**
   * @brief Converts a SISO entity enumeration to text.
   * @param sisoEntityEnumeration Enumeration to convert.
   * @return String representation of the enumeration.
   */
  UFUNCTION(BlueprintPure, meta = (DisplayName = "SisoEntityEnumeration To String", CompactNodeTitle = "->", BlueprintAutocast), Category = "Cigi|Conversions")
  static FString Conv_SisoEntityEnumerationToString(FSisoID sisoEntityEnumeration);
  /**
   * @brief Converts component data to text.
   * @param componentData Component data to convert.
   * @return String representation of the component data.
   */
  UFUNCTION(BlueprintPure, meta = (DisplayName = "ComponentData To String", CompactNodeTitle = "->", BlueprintAutocast), Category = "Cigi|Conversions")
  static FString Conv_ComponentDataToString(FComponentData componentData);
  /**
   * @brief Converts a component message to text.
   * @param componentMessage Component message to convert.
   * @return String representation of the component message.
   */
  UFUNCTION(BlueprintPure, meta = (DisplayName = "ComponentMessage To String", CompactNodeTitle = "->", BlueprintAutocast), Category = "Cigi|Conversions")
  static FString Conv_ComponentMessageToString(FComponentMessage componentMessage);
  /**
   * @brief Converts a component class to text.
   * @param componentClass Component class to convert.
   * @return String representation of the component class.
   */
  UFUNCTION(BlueprintPure, meta = (DisplayName = "ComponentClass To String", CompactNodeTitle = "->", BlueprintAutocast), Category = "Cigi|Conversions")
  static FString Conv_ComponentClassToString(ComponentClass componentClass);

  // --- Coordinate Conversions ---

  /** Converts from Geodetic coordinates (Longitude, Latitude, Altitude) to Unreal coordinates (X/Y/Z in cm).
   * If a GeoReferencingSystem actor exists, it does coordinate conversion (using GeographicToEngine).
   * Otherwise, this defaults to a local tangent plane conversion using the JSON-defined origin point of the loaded CIGI database. */
  UFUNCTION(BlueprintPure, Category = "Cigi|Coordinate")
  static void GeodeticToEngine(FVector LonLatAlt, /*out*/ FVector& XYZ);

  /** Converts from Unreal coordinates (X/Y/Z in cm) to Geodetic coordinates (Longitude, Latitude, Altitude).
   * If a GeoReferencingSystem actor exists, it does coordinate conversion (using EngineToGeographic).
   * Otherwise, this defaults to a local tangent plane conversion using the JSON-defined origin point of the loaded CIGI database. */
  UFUNCTION(BlueprintPure, Category = "Cigi|Coordinate")
  static void EngineToGeodetic(FVector XYZ, /*out*/ FVector& LonLatAlt);

  /** Returns the geodetic origin point (Longitude, Latitude, Altitude) being used by Cigi.
   * If there is a GeoReferencingSystem, its origin is used. Otherwise this returns the local tangent plane origin. */
  UFUNCTION(BlueprintPure, Category = "Cigi|Coordinate|LTP")
  static FVector GetCigiOrigin();

  /** Takes in a geodetic point (Longitude, Latitude, Altitude) and sets that as the new origin point for LTP conversions.
   * Note: This is done automatically when a database is loaded if the config.json file has "lon", "lat", and "alt" values.
   * (Setting the origin point in UnrealCigi.config.json is the recommended method)*/
  UFUNCTION(BlueprintCallable, Category = "Cigi|Coordinate|LTP")
  static void SetLocalTangentPlaneOrigin(FVector LonLatAlt);

  /** Converts from Geodetic coordinates (Longitude, Latitude, Altitude) to Unreal coordinates (X/Y/Z in cm) using an LTP conversion.
   * LTP (Local Tangent Plane) uses an origin Lon/Lat/Alt point to convert between geodetic and local space.
   * The origin point can be read from GetLTPOrigin and it can be changed with SetLocalTangentPlaneOrigin */
  UFUNCTION(BlueprintPure, Category = "Cigi|Coordinate|LTP")
  static void GeodeticToLocalTangentPlane(FVector LonLatAlt, /*out*/ FVector& XYZ);

  /** Converts from Unreal coordinates (X/Y/Z in cm) to Geodetic coordinates (Longitude, Latitude, Altitude) using an LTP conversion.
   * LTP (Local Tangent Plane) uses an origin Lon/Lat/Alt point to convert between geodetic and local space.
   * The origin point can be read from GetLTPOrigin and it can be changed with SetLocalTangentPlaneOrigin */
  UFUNCTION(BlueprintPure, Category = "Cigi|Coordinate|LTP")
  static void LocalTangentPlaneToGeodetic(FVector XYZ, /*out*/ FVector& LonLatAlt);
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026