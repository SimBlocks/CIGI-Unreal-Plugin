//Copyright SimBlocks LLC 2016-2026
/**
 * @file SymbolConfig.h
 * @brief Defines configuration for symbol templates used in CIGI symbol rendering in Unreal Engine.
 *
 * This header provides:
 * - USymbolConfig: UObject class for storing symbol template configuration, supporting initialization from JSON.
 * - Properties for symbol type, drawing style, primitive topology, line settings, vertices, radii, and angles.
 * - Utility functions for initialization, validation, and summary output.
 *
 * Usage:
 * - Use USymbolConfig to store and initialize symbol template data, typically parsed from JSON.
 * - Supports both circle and polygon symbol types, with line and stipple settings.
 * - Provides validation and summary functions for symbol configuration.
 */

#pragma once

#include "ModuleAPI.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SymbolLib/SymbolTypes.h"
#include "SymbolConfig.generated.h"

/**
 * @brief Configuration for one textured-circle element in a symbol template.
 */
USTRUCT()
struct FTexturedCircleTemplateElement
{
  GENERATED_BODY()

  UPROPERTY()
  FVector2D centerUV = FVector2D::ZeroVector;

  UPROPERTY()
  float radius = 0.0f;

  UPROPERTY()
  float innerRadius = 0.0f;

  UPROPERTY()
  FVector2D angles = FVector2D::ZeroVector;

  UPROPERTY()
  FVector2D centerTextureST = FVector2D::ZeroVector;

  UPROPERTY()
  float textureMapRadius = 0.0f;

  UPROPERTY()
  float textureMapRotation = 0.0f;
};

/**
 * @brief Configuration for one textured-polygon vertex in a symbol template.
 */
USTRUCT()
struct FTexturedPolygonTemplateVertex
{
  GENERATED_BODY()

  UPROPERTY()
  FVector2D uv = FVector2D::ZeroVector;

  UPROPERTY()
  FVector2D textureCoordinateST = FVector2D::ZeroVector;
};

/**
 * @class USymbolConfig
 * @brief Stores configuration for a symbol template used in CIGI symbol rendering.
 *
 * Supports circle, polygon, textured-circle, and textured-polygon symbols, with line and stipple settings.
 * Provides initialization from JSON and utility functions for validation and summary.
 */
UCLASS()

class MODULE_API USymbolConfig : public UObject
{
  GENERATED_BODY()
public:
  /**
   * @brief The ID of this symbol template. Used by Symbol Clone packets to spawn this template.
   */
  UPROPERTY()
  int32 templateID;

  /**
   * @brief Reflected integer backing value for the JSON symbol name.
   *
   * JSON uses `circle`, `polygon`, `texturedCircle`, or `texturedPolygon`.
   * The parsed integer is retained for Unreal reflection and serialization.
   */
  UPROPERTY()
  int32 symbol;

  /**
   * @brief Circle drawing style value stored as int32 for Unreal reflection.
   *
   * JSON uses `drawingStyle`: `line` or `fill`. The stored value uses the SDK
   * `EDrawingStyle` values: 0 = line and 1 = fill.
   */
  UPROPERTY()
   int32 drawingStyle = -1;

  /**
   * @brief Polygon primitive topology value stored as int32 for Unreal reflection.
   *
   * JSON uses `primitiveType`: `point`, `line`, `lineStrip`, `lineLoop`,
   * `triangle`, `triangleStrip`, or `triangleFan`. The stored value uses the
   * SDK `EPrimitiveType` values: 0 through 6 respectively.
   */
  UPROPERTY()
  int32 primitiveType = -1;

  /**
   * @brief If this symbol is a line, this defines the width of that line.
   */
  UPROPERTY()
  int32 lineWidth;

  /**
   * @brief If this symbol is a line, this defines the stipple pattern input for a Symbol Circle or Symbol Polygon packet.
   */
  UPROPERTY()
  int32 stipple;

  /**
   * @brief If this symbol is a line, this defines the length of the stipple pattern.
   */
  UPROPERTY()
  int32 stippleLength;

  /**
   * @brief The list of center points (circle) or vertices (polygon) for this symbol.
   */
  UPROPERTY()
  TArray<FVector2D> vertices;

  /**
   * @brief A list of inner and outer radii for each shape (circles only).
   */
  UPROPERTY()
  TArray<FVector2D> radii;

  /**
   * @brief A list of start and end angles for each shape (circles only).
   */
  UPROPERTY()
  TArray<FVector2D> angles;

  /** @brief Texture identifier used by textured circle and textured polygon templates. */
  UPROPERTY()
  int32 textureID = -1;

  /** @brief Texture filter mode value used by textured templates. */
  UPROPERTY()
  int32 textureFilterMode = -1;

  /** @brief Texture wrap mode value used by textured templates. */
  UPROPERTY()
  int32 textureWrapMode = -1;

  /** @brief Textured-circle elements for textured-circle templates. */
  UPROPERTY()
  TArray<FTexturedCircleTemplateElement> texturedCircles;

  /** @brief Textured-polygon vertices for textured-polygon templates. */
  UPROPERTY()
  TArray<FTexturedPolygonTemplateVertex> texturedPolygonVertices;

  /**
   * @brief Default constructor.
   */
  USymbolConfig();

  /**
   * @brief Initializes this symbol information with all of the data parsed from the JSON file.
   * @param _templateID The template ID.
    * @param _symbol Parsed symbol value.
    * @param _drawingStyle Parsed circle drawing style value.
    * @param _primitiveType Parsed polygon primitive type value.
   * @param _lineWidth Line width.
   * @param _stipple Stipple pattern.
   * @param _stippleLength Stipple length.
   * @param _vertices List of vertices or center points.
   * @param _radii List of inner/outer radii.
   * @param _angles List of start/end angles.
   */
  UFUNCTION()
   void InitJSON(int32 _templateID, int32 _symbol, int32 _drawingStyle, int32 _primitiveType, int32 _lineWidth, int32 _stipple, int32 _stippleLength, TArray<FVector2D> _vertices, TArray<FVector2D> _radii, TArray<FVector2D> _angles);

  /**
   * @brief Initializes this symbol information by copying from another config.
   * @param copyFrom The config to copy from.
   */
  void InitJSON(USymbolConfig& copyFrom);

  /** @brief Returns the configured SDK circle drawing style. */
  sbio::symbol::EDrawingStyle GetDrawingStyle() const;

  /** @brief Returns the configured SDK polygon primitive type. */
  sbio::symbol::EPrimitiveType GetPrimitiveType() const;

  /** @brief Returns the configured SDK symbol type. */
  sbio::symbol::ESymbolType GetSymbolType() const;

  /**
   * @brief Returns a string summarizing the key information about this symbol.
   * @return Summary string.
   */
  FString ToString() const;

  /**
   * @brief Returns an empty string on success, or an error message on failure.
   * @return Validation result string.
   */
  FString ValidateSymbol() const;

  /**
   * @brief Returns true if this symbol is a line.
   * @return True if line, false otherwise.
   */
  bool IsLine() const;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026