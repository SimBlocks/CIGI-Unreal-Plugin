//Copyright SimBlocks LLC 2016-2026
/**
 * @file SlateCigiWidget.h
 * @brief Defines the SSlateCigiWidget class for custom CIGI symbol surface rendering in Unreal Engine Slate UI.
 *
 * This header provides:
 * - The SSlateCigiWidget class, derived from SCompoundWidget, for drawing CIGI symbol surfaces and their symbols.
 * - Paint helper functions for rendering circles, polygons, textured geometry, and text.
 * - Integration with Unreal's Slate UI system for advanced symbol rendering.
 *
 * Usage:
 * - SSlateCigiWidget is constructed with a SurfaceID and used by UCigiWidget to display CIGI symbol surfaces.
 * - Override OnPaint to implement custom drawing logic for symbols.
 * - Use PaintCircle, PaintPolygon, PaintTexturedCircle, PaintTexturedPolygon, and PaintText for geometry-specific rendering.
 */

#pragma once

#include "CoreMinimal.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ModuleAPI.h"
#include "SymbolLib/Symbol.h"
#include "SymbolLib/SymbolTypes.h"
#include "UnrealCigi_Declarations.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCigiSlateSymbols, Log, All)

// forward declarations
class SlateCustomVerts;

/**
 * @class SSlateCigiWidget
 * @brief Slate widget for rendering a CIGI symbol surface and its symbols.
 *
 * SSlateCigiWidget draws symbol geometry (circles, polygons, text, etc.) for a given surface.
 * Used by UCigiWidget for custom symbol rendering in Unreal UI.
 */
class MODULE_API SSlateCigiWidget : public SCompoundWidget
{
public:
  /**
   * @struct FArguments
   * @brief Slate arguments for widget construction.
   * @param SurfaceID The symbol surface ID to render.
   */
  SLATE_BEGIN_ARGS(SSlateCigiWidget)
  {
  }
  SLATE_ARGUMENT(int32, SurfaceID)
  SLATE_END_ARGS()

  /**
   * @brief Called once when the widget is created. Initializes with SurfaceID.
   * @param InArgs Slate construction arguments.
   */
  void Construct(const FArguments& InArgs);

  /**
   * @brief Called every frame to draw the widget.
   * Controls logic for displaying symbol surfaces and all supported symbols.
   * @param Args Paint arguments.
   * @param AllottedGeometry Geometry for widget layout.
   * @param MyCullingRect Culling rectangle.
   * @param OutDrawElements Output draw elements.
   * @param LayerId Drawing layer.
   * @param InWidgetStyle Widget style.
   * @param bParentEnabled Is parent enabled.
   * @return Next layer ID.
   */
  int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const;

protected:
  /**
   * @var int32 SurfaceID
   * @brief Symbol surface ID.
   */
  int32 SurfaceID = -1;
  /**
   * @var int32 PaintID
   * @brief Paint identifier.
   */
  int32 PaintID = -1;
  /**
   * @var int32 PaintCounter
   * @brief Paint call counter.
   */
  int32 PaintCounter = -1;

private:
  /**
   * @brief Paints a circle symbol.
   * @param symbol Pointer to the symbol to paint.
   * @param verts Reference to the vertex helper.
   */
  void PaintCircle(sbio::symbol::CSymbol* symbol, SlateCustomVerts& verts) const;

  /**
   * @brief Paints a polygon symbol.
   * @param symbol Pointer to the symbol to paint.
   * @param verts Reference to the vertex helper.
   */
  void PaintPolygon(sbio::symbol::CSymbol* symbol, SlateCustomVerts& verts) const;

  /**
   * @brief Paints a textured circle symbol.
   * @param symbol Pointer to the symbol to paint.
   * @param verts Reference to the vertex helper.
   */
  void PaintTexturedCircle(sbio::symbol::CSymbol* symbol, SlateCustomVerts& verts) const;

  /**
   * @brief Paints a textured polygon symbol.
   * @param symbol Pointer to the symbol to paint.
   * @param verts Reference to the vertex helper.
   */
  void PaintTexturedPolygon(sbio::symbol::CSymbol* symbol, SlateCustomVerts& verts) const;

  /**
   * @brief Paints a text symbol.
   * @param symbol Pointer to the symbol to paint.
   * @param symbolID Symbol identifier.
   * @param surface Pointer to the symbol surface.
   * @param AllottedGeometry Geometry for widget layout.
   * @param OutDrawElements Output draw elements.
   * @param LayerId Drawing layer.
   */
  void PaintText(sbio::symbol::CSymbol* symbol, sbio::symbol::SymbolID symbolID, sbio::unrealcigi::FUnrealSymbolSurface* surface, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;
};

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026