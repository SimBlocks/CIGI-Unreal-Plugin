//Copyright SimBlocks LLC 2016-2026
/**
 * @file SlateCustomVerts.h
 * @brief Support classes for custom vertex drawing in Unreal Slate UI for CIGI symbol rendering.
 *
 * This header provides:
 * - VertexTransform: Structure for applying scale, rotation, and offset to vertices in UV space.
 * - SlateCustomVerts: Class for constructing and transforming custom vertices and indices for use with FSlateDrawElement::MakeCustomVerts.
 *
 * Usage:
 * - Use SlateCustomVerts within SCompoundWidget::OnPaint to draw custom geometry for CIGI symbols.
 * - Set up UV grids, apply transformations, and add vertices, triangles, quads, circles, and lines for advanced symbol rendering.
 * - Handles conversion from CIGI UV coordinates to Unreal screen coordinates and fixes vertex ordering for Slate.
 */

#pragma once

#include "CoreMinimal.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ModuleAPI.h"

class UTexture2D;

DECLARE_LOG_CATEGORY_EXTERN(LogCigiSlateVerts, Log, All)

/**
 * @struct VertexTransform
 * @brief Structure for applying scale, rotation, and offset to vertices in UV space.
 *
 * Transforms are applied in this order: Scale, then Rotation, then Offset.
 */
struct VertexTransform
{
  /** Offset (in UV units) applied during transform. */
  FVector2D Offset;
  /** Rotation (in degrees) applied during transform. */
  float Rotation;
  /** Scale (in UV units) applied during transform. */
  FVector2D Scale;
  /** If true, parent scale is applied (CIGI usually ignores parent scale). */
  bool ApplyParentScale;

  /**
   * @brief Constructs a VertexTransform with all fields.
   * @param offset Offset in UV units.
   * @param rotation Rotation in degrees.
   * @param scale Scale in UV units.
   * @param applyParentScale Whether to apply parent scale.
   */
  VertexTransform(FVector2D offset, float rotation, FVector2D scale, bool applyParentScale = false);
};

/**
 * @class SlateCustomVerts
 * @brief Support class for using FSlateDrawElement::MakeCustomVerts in Slate UI.
 *
 * Handles transformations from CIGI UV coordinates to Unreal screen coordinates.
 * Manages construction of polygons, vertex ordering, and geometry for symbol rendering.
 *
 * - Use Vertices and Indices arrays for custom Slate drawing.
 * - Apply multiple transforms for child/parent symbol relationships.
 * - Set up UV grids for mapping coordinates to screen space.
 * - Add vertices, triangles, quads, circles, and lines for symbol geometry.
 */
class MODULE_API SlateCustomVerts
{
public:
  /** Constructed vertices for drawing. */
  TArray<FSlateVertex> Vertices;
  /** Constructed indices for drawing. */
  TArray<SlateIndex> Indices;
  /** Transforms applied to each vertex (child/parent relationships). */
  TArray<VertexTransform> Transforms;
  /** Texture resource used by textured custom vertices. */
  UTexture2D* Texture = nullptr;

  /**
   * @brief Constructs a SlateCustomVerts object with geometry position and size.
   * @param allocatedGeometryPos Absolute position of the geometry.
   * @param allocatedGeometrySize Absolute size of the geometry.
   */
  SlateCustomVerts(FVector2D allocatedGeometryPos, FVector2D allocatedGeometrySize);

  /**
   * @brief Sets up the UV grid for mapping coordinates to screen space.
   * @param minUV Minimum UV coordinates.
   * @param maxUV Maximum UV coordinates.
   * @param upPositive True if up is positive, false if negative.
   */
  void SetUVGrid(FVector2D minUV, FVector2D maxUV, bool upPositive = true);

  /**
   * @brief Removes the UV grid mapping.
   */
  void RemoveUVGrid();

  /**
   * @brief Converts UV coordinates to screen coordinates.
   * @param uv UV coordinates.
   * @return Screen coordinates as FVector2D.
   */
  FVector2D UVToScreen(FVector2D uv) const;

  /**
   * @brief Applies all transforms to a UV coordinate.
   * @param uv UV coordinates.
   * @return Transformed UV coordinates.
   */
  FVector2D ApplyTransform(FVector2D uv);

  /**
   * @brief Creates a Slate render transform from the current transforms.
   * @return Slate render transform.
   */
  FSlateRenderTransform MakeRenderTransform();

  /**
   * @brief Determines if three points are ordered clockwise.
   * @param a First point.
   * @param b Second point.
   * @param c Third point.
   * @return 1 if clockwise, -1 if counterclockwise, 0 if colinear.
   */
  int IsClockwise(FVector2D a, FVector2D b, FVector2D c);

  /**
   * @brief Calculates a position on a circle.
   * @param angle Angle in degrees.
   * @param radius Circle radius.
   * @param center Center of the circle.
   * @return Position on the circle as FVector2D.
   */
  FVector2D PosOnCircle(float angle, float radius = 1, FVector2D center = FVector2D(0, 0));

  /**
   * @brief Reads a stipple pattern bit.
   * @param stipplePattern Stipple pattern.
   * @param index Bit index.
   * @return True if bit is set, false otherwise.
   */
  bool ReadStipple(uint16_t stipplePattern, int index);

  /**
   * @brief Adds a vertex to the drawing list.
   * @param color Vertex color.
   * @param pos Vertex position.
   */
  void AddVertex(FColor color, FVector2D pos);

  /**
   * @brief Adds a triangle to the drawing list.
   * @param color Triangle color.
   * @param a First vertex.
   * @param b Second vertex.
   * @param c Third vertex.
   */
  void AddTriangle(FColor color, FVector2D a, FVector2D b, FVector2D c);

  /** Adds a triangle with per-vertex texture ST coordinates. */
  void AddTexturedTriangle(FColor color, FVector2D a, FVector2D aST, FVector2D b, FVector2D bST, FVector2D c, FVector2D cST);

  /**
   * @brief Adds a quad to the drawing list.
   * @param color Quad color.
   * @param a First vertex.
   * @param b Second vertex.
   * @param c Third vertex.
   * @param d Fourth vertex.
   */
  void AddQuad(FColor color, FVector2D a, FVector2D b, FVector2D c, FVector2D d);

  /**
   * @brief Adds a circle or arc to the drawing list.
   * @param color Circle color.
   * @param center Center position.
   * @param outerRadius Outer radius.
   * @param innerRadius Inner radius (default 0).
   * @param startAngle Start angle in degrees (default 0).
   * @param endAngle End angle in degrees (default 360).
   * @param stipplePattern Stipple pattern (default 0).
   * @param stippleLength Stipple length (default -1).
   * @param segmentDegrees Segment angle in degrees (default -1).
   */
  void AddCircle(FColor color, FVector2D center, float outerRadius, float innerRadius = 0, float startAngle = 0, float endAngle = 360, uint16_t stipplePattern = 0, float stippleLength = -1, int segmentDegrees = -1);

  /**
   * @brief Adds a line or polyline to the drawing list.
   * @param color Line color.
   * @param points Array of points.
   * @param connected True if points are connected.
   * @param loop True if line should loop.
   * @param lineWidth Width of the line.
   * @param stipplePattern Stipple pattern (default 0).
   * @param stippleLength Stipple length (default -1).
   */
  void AddLine(FColor color, TArray<FVector2D> points, bool connected, bool loop, float lineWidth, uint16_t stipplePattern = 0, float stippleLength = -1);

protected:
  void AddTexturedVertex(FColor color, FVector2D pos, FVector2D textureST);

private:
  /** Absolute position of the geometry. */
  FVector2D AllocatedGeometryPos;
  /** Absolute size of the geometry. */
  FVector2D AllocatedGeometrySize;
  /** The Aspect Ratio of AllocatedGeometrySize. One axis must be ==1.0, the other must be >=1.0 */
  FVector2D AspectRatioScreen;
  /** Offset for UV to screen conversion. */
  FVector2D Offset;
  /** Scale for UV to screen conversion. */
  FVector2D Scale;
  /** True if up is positive, false if negative. */
  bool UpPositive;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026