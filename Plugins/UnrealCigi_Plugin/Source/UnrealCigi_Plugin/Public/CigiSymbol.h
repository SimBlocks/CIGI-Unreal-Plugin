//Copyright SimBlocks LLC 2016-2026
/**
 * @file CigiSymbol.h
 * @brief Defines Unreal render state for SymbolLib-managed CIGI symbols.
 *
 * This header provides:
 * - SymbolSurfaceType: Enum for symbol surface rendering modes (view, billboard, world).
 * - FUnrealSymbolSurface: Unreal widget and transform state for a SymbolLib surface.
 *
 * Usage:
 * - Use FUnrealSymbolSurface to describe how and where symbols are rendered.
 * - Use SymbolLib as the authoritative source for CIGI-domain symbol geometry and packet state.
 */

#pragma once

#include "ModuleAPI.h"
#include "EngineLib/ImageGeneratorMessages.h"
#include "SymbolLib/Symbol.h"
#include "SymbolLib/SymbolTypes.h"

class UCigiWidget;

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @brief Stores a textured symbol vertex position and texture coordinates.
     */
    struct FTexturedSymbolVertex
    {
      FVector2D Position;///< Position of the vertex
      FVector2D TextureST;///< Texture coordinates of the vertex
    };

    /**
     * @brief Caches triangulated geometry and texture state for a textured circle.
     */
    struct FTexturedCircleGeometryCache
    {
      uint64 nGeometryRevision = 1;///< Revision number of the geometry
      uint64 nCachedGeometryRevision = 0;///< Cached geometry revision number
      TArray<FTexturedSymbolVertex> TriangleVertices;///< Vertices of the triangulated geometry
      sbio::TextureID TextureID = sbio::UnknownTextureID;///< ID of the texture
      sbio::symbol::ETextureFilterMode FilterMode = sbio::symbol::ETextureFilterMode::UNKNOWN;///< Filter mode of the texture
      sbio::symbol::ETextureWrapMode WrapMode = sbio::symbol::ETextureWrapMode::UNKNOWN;///< Wrap mode of the texture
    };

    /**
     * @brief Caches triangulated geometry for an untextured symbol.
     */
    struct FSymbolGeometryCache
    {
      uint64 nGeometryRevision = 1;///< Revision number of the geometry
      uint64 nCachedGeometryRevision = 0;///< Cached geometry revision number
      TArray<FVector2D> TriangleVertices;///< Vertices of the triangulated geometry
    };

    /**
     * @brief Caches triangulated geometry and texture state for a textured polygon.
     */
    struct FTexturedPolygonGeometryCache
    {
      uint64 nGeometryRevision = 1;///< Revision number of the geometry
      uint64 nCachedGeometryRevision = 0;///< Cached geometry revision number
      TArray<FTexturedSymbolVertex> TriangleVertices;///< Vertices of the triangulated geometry
      sbio::TextureID TextureID = sbio::UnknownTextureID;///< ID of the texture
      sbio::symbol::ETextureFilterMode FilterMode = sbio::symbol::ETextureFilterMode::UNKNOWN;///< Filter mode of the texture
      sbio::symbol::ETextureWrapMode WrapMode = sbio::symbol::ETextureWrapMode::UNKNOWN;///< Wrap mode of the texture
    };

    /**
     * @brief Caches rendered text state and measured dimensions for a symbol.
     */
    struct FSymbolTextCache
    {
      uint64 TextRevision = 1;///< Revision number of the text
      uint64 CachedTextRevision = 0;///< Cached text revision number
      FString DisplayString;///< The string to be displayed
      sbio::FontID FontID = sbio::UnknownFontID;///< ID of the font
      float fFontSize = 0;///< Size of the font
      sbio::symbol::ETextAlignment Alignment = sbio::symbol::ETextAlignment::UNKNOWN;///< Alignment of the text
      sbio::symbol::ETextOrientation Orientation = sbio::symbol::ETextOrientation::UNKNOWN;///< Orientation of the text
      FVector2D MeasuredSize = FVector2D::ZeroVector;///< Measured size of the text
      bool bMeasurementValid = false;///< Is the measurement valid
    };

    /**
     * @enum SymbolSurfaceType
     * @brief Enumerates symbol surface types for rendering.
     * @note UNKNOWN: Surface not yet created.
     *       VIEW: Surface attached to a ViewID.
     *       BILLBOARD: Surface attached to an entity, always faces viewer.
     *       WORLD: Surface attached to an entity, rotated in world space.
     */
    enum SymbolSurfaceType
    {
      UNKNOWN,///< Surface not yet created
      VIEW,///< Surface attached to a ViewID
      BILLBOARD,///< Surface attached to an entity, always faces viewer
      WORLD///< Surface attached to an entity, rotated in world space
    };

    /**
     * @struct FUnrealSymbolSurface
     * @brief Describes a symbol surface for rendering symbols.
     * @details Contains surface ID, type, UV bounds, attachment, size, rotation, widget, and symbol list.
     */
    struct FUnrealSymbolSurface
    {
      sbio::symbol::SymbolSurfaceID SurfaceID = sbio::symbol::UnknownSymbolSurfaceID;///< Unique surface ID
      SymbolSurfaceType Type = SymbolSurfaceType::UNKNOWN;///< Surface type
      FVector2D MinUV;///< UV coordinates of left/bottom corner
      FVector2D MaxUV;///< UV coordinates of right/top corner
      bool Enabled = true;///< Is surface enabled
      int AttachID = -1;///< ViewID or EntityID attached
      FVector Offset;///< Offset from attached object
      FVector2D Size;///< Width/height of surface
      FQuat Rotation;///< Rotation (for world surfaces)
      UCigiWidget* Widget = nullptr;///< Widget rendering this surface
      SymbolSurfaceType WidgetType = SymbolSurfaceType::UNKNOWN;///< Current widget display type

      /**
       * @brief Constructs Unreal render state for the given surface ID.
       * @param surfaceID The unique surface ID.
       */
      FUnrealSymbolSurface(sbio::symbol::SymbolSurfaceID surfaceID);

      /**
       * @brief Returns a string representation of the symbol surface.
       * @return String describing the surface.
       */
      FString ToString() const;
    };

  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026