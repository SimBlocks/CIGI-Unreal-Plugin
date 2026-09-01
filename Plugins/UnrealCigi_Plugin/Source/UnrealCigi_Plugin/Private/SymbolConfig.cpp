//Copyright SimBlocks LLC 2016-2026

#include "SymbolConfig.h"

USymbolConfig::USymbolConfig()
{
  templateID = -1;
  symbol = -1;
  drawingStyle = -1;
  primitiveType = -1;
  lineWidth = -1;
  stipple = -1;
  stippleLength = -1;
  vertices = TArray<FVector2D>();
  radii = TArray<FVector2D>();
  angles = TArray<FVector2D>();
  textureID = -1;
  textureFilterMode = -1;
  textureWrapMode = -1;
  texturedCircles = TArray<FTexturedCircleTemplateElement>();
  texturedPolygonVertices = TArray<FTexturedPolygonTemplateVertex>();
}

sbio::symbol::EDrawingStyle USymbolConfig::GetDrawingStyle() const
{
  if (drawingStyle == static_cast<int32>(sbio::symbol::EDrawingStyle::LINE))
  {
    return sbio::symbol::EDrawingStyle::LINE;
  }
  if (drawingStyle == static_cast<int32>(sbio::symbol::EDrawingStyle::FILL))
  {
    return sbio::symbol::EDrawingStyle::FILL;
  }

  return sbio::symbol::EDrawingStyle::UNKNOWN;
}

sbio::symbol::EPrimitiveType USymbolConfig::GetPrimitiveType() const
{
  if (primitiveType < static_cast<int32>(sbio::symbol::EPrimitiveType::POINT) || primitiveType > static_cast<int32>(sbio::symbol::EPrimitiveType::TRIANGLE_FAN))
  {
    return sbio::symbol::EPrimitiveType::UNKNOWN;
  }

  return static_cast<sbio::symbol::EPrimitiveType>(primitiveType);
}

sbio::symbol::ESymbolType USymbolConfig::GetSymbolType() const
{
  switch (symbol)
  {
    case 1:
      return sbio::symbol::ESymbolType::CIRCLE;
    case 2:
      return sbio::symbol::ESymbolType::POLYGON;
    case 3:
      return sbio::symbol::ESymbolType::TEXTURED_CIRCLE;
    case 4:
      return sbio::symbol::ESymbolType::TEXTURED_POLYGON;
    default:
      return sbio::symbol::ESymbolType::UNKNOWN;
  }
}

void USymbolConfig::InitJSON(int32 _templateID, int32 _symbol, int32 _drawingStyle, int32 _primitiveType, int32 _lineWidth, int32 _stipple, int32 _stippleLength, TArray<FVector2d> _vertices, TArray<FVector2d> _radii, TArray<FVector2d> _angles)
{
  templateID = _templateID;
  symbol = _symbol;
  drawingStyle = _drawingStyle;
  primitiveType = _primitiveType;
  lineWidth = _lineWidth;
  stipple = _stipple;
  stippleLength = _stippleLength;
  vertices = _vertices;
  radii = _radii;
  angles = _angles;
}

void USymbolConfig::InitJSON(USymbolConfig& copyFrom)
{
  templateID = copyFrom.templateID;
  symbol = copyFrom.symbol;
  drawingStyle = copyFrom.drawingStyle;
  primitiveType = copyFrom.primitiveType;
  lineWidth = copyFrom.lineWidth;
  stipple = copyFrom.stipple;
  stippleLength = copyFrom.stippleLength;
  vertices = copyFrom.vertices;
  radii = copyFrom.radii;
  angles = copyFrom.angles;
  textureID = copyFrom.textureID;
  textureFilterMode = copyFrom.textureFilterMode;
  textureWrapMode = copyFrom.textureWrapMode;
  texturedCircles = copyFrom.texturedCircles;
  texturedPolygonVertices = copyFrom.texturedPolygonVertices;
}

FString USymbolConfig::ToString() const
{
  return FString::Printf(TEXT("{templateID=%d, symbol=%d, drawingStyle=%d, primitiveType=%d, shapes=%d}"), templateID, symbol, drawingStyle, primitiveType, vertices.Num());
}

FString USymbolConfig::ValidateSymbol() const
{
  if (templateID <= 0 || templateID > MAX_uint16)
  {
    return FString::Printf(TEXT("Invalid templateID %d"), templateID);
  }

  const auto validateTextureSettings = [this]() -> FString
  {
    if (textureID <= 0 || textureID > MAX_uint16)
    {
      return FString::Printf(TEXT("Invalid textureID %d"), textureID);
    }

    const sbio::symbol::ETextureFilterMode filterMode = static_cast<sbio::symbol::ETextureFilterMode>(textureFilterMode);
    if (filterMode != sbio::symbol::ETextureFilterMode::NEAREST && filterMode != sbio::symbol::ETextureFilterMode::LINEAR)
    {
      return FString::Printf(TEXT("Invalid textureFilterMode %d"), textureFilterMode);
    }

    const sbio::symbol::ETextureWrapMode wrapMode = static_cast<sbio::symbol::ETextureWrapMode>(textureWrapMode);
    if (wrapMode != sbio::symbol::ETextureWrapMode::REPEAT && wrapMode != sbio::symbol::ETextureWrapMode::CLAMP)
    {
      return FString::Printf(TEXT("Invalid textureWrapMode %d"), textureWrapMode);
    }

    return FString();
  };

  // Circles
  if (GetSymbolType() == sbio::symbol::ESymbolType::CIRCLE)
  {
    const sbio::symbol::EDrawingStyle drawingStyleType = GetDrawingStyle();

    if (vertices.IsEmpty())
    {
      return TEXT("Circle has no elements");
    }
    if (vertices.Num() != radii.Num() || vertices.Num() != angles.Num())
    {
      return FString::Printf(TEXT("Circle geometry array sizes do not match: vertices=%d, radii=%d, angles=%d"), vertices.Num(), radii.Num(), angles.Num());
    }
    for (int32 i = 0; i < radii.Num(); ++i)
    {
      if (radii[i].X <= 0.0f)
      {
        return FString::Printf(TEXT("Invalid circle outer radius at element %d"), i);
      }
      if (radii[i].Y < 0.0f || radii[i].Y > radii[i].X)
      {
        return FString::Printf(TEXT("Invalid circle inner radius at element %d"), i);
      }
    }

     // Drawing Style == Line
    if (drawingStyleType == sbio::symbol::EDrawingStyle::LINE)
    {
      if (lineWidth < 0)
      {
        return FString::Printf(TEXT("Invalid circle lineWidth %d"), lineWidth);
      }
      if (stipple < 0 || stipple > MAX_uint16)
      {
        return FString::Printf(TEXT("Invalid circle stipple %d"), stipple);
      }
      if (stippleLength < 0)
      {
        return FString::Printf(TEXT("Invalid circle stippleLength %d"), stippleLength);
      }
      return "";
    }
    
    // Drawing Style == Fill
     if (drawingStyleType == sbio::symbol::EDrawingStyle::FILL)
    {
      return "";
    }
    else
    {
       return FString::Printf(TEXT("Invalid circle drawingStyle %d"), drawingStyle);
    }
  }
  // Polygons
  else if (GetSymbolType() == sbio::symbol::ESymbolType::POLYGON)
  {
    const sbio::symbol::EPrimitiveType primitiveTypeValue = GetPrimitiveType();

    // Polygon Type == Point
    if (primitiveTypeValue == sbio::symbol::EPrimitiveType::POINT)
    {
      return "";
    }
    
    // Polygon Type == Line, Line Strip, or Line Loop
     if (primitiveTypeValue == sbio::symbol::EPrimitiveType::LINE || primitiveTypeValue == sbio::symbol::EPrimitiveType::LINE_STRIP || primitiveTypeValue == sbio::symbol::EPrimitiveType::LINE_LOOP)
    {
      if (lineWidth < 0)
      {
        return FString::Printf(TEXT("Invalid polygon lineWidth %d"), lineWidth);
      }
      if (stipple < 0 || stipple > MAX_uint16)
      {
        return FString::Printf(TEXT("Invalid polygon stipple %d"), stipple);
      }
      if (stippleLength < 0)
      {
        return FString::Printf(TEXT("Invalid polygon stippleLength %d"), stippleLength);
      }
      return "";
    }
    // Polygon Type == Triangle, Triangle Strip, or Triangle Fan
    if (primitiveTypeValue == sbio::symbol::EPrimitiveType::TRIANGLE || primitiveTypeValue == sbio::symbol::EPrimitiveType::TRIANGLE_STRIP || primitiveTypeValue == sbio::symbol::EPrimitiveType::TRIANGLE_FAN)
    {
      return "";
    }
    else
    {
       return FString::Printf(TEXT("Invalid polygon primitiveType %d"), primitiveType);
    }
  }

  // Textured circles
  else if (GetSymbolType() == sbio::symbol::ESymbolType::TEXTURED_CIRCLE)
  {
    const FString textureError = validateTextureSettings();
    if (!textureError.IsEmpty())
    {
      return textureError;
    }
    if (texturedCircles.IsEmpty())
    {
      return TEXT("Textured circle has no elements");
    }
    for (int32 i = 0; i < texturedCircles.Num(); ++i)
    {
      if (texturedCircles[i].radius <= 0.0f)
      {
        return FString::Printf(TEXT("Invalid textured circle radius at element %d"), i);
      }
      if (texturedCircles[i].innerRadius < 0.0f || texturedCircles[i].innerRadius > texturedCircles[i].radius)
      {
        return FString::Printf(TEXT("Invalid textured circle innerRadius at element %d"), i);
      }
      if (texturedCircles[i].textureMapRadius <= 0.0f)
      {
        return FString::Printf(TEXT("Invalid textureMapRadius at element %d"), i);
      }
    }
    return "";
  }

  // Textured polygons
  else if (GetSymbolType() == sbio::symbol::ESymbolType::TEXTURED_POLYGON)
  {
    const sbio::symbol::EPrimitiveType primitiveTypeValue = GetPrimitiveType();
    if (primitiveTypeValue != sbio::symbol::EPrimitiveType::TRIANGLE && primitiveTypeValue != sbio::symbol::EPrimitiveType::TRIANGLE_STRIP && primitiveTypeValue != sbio::symbol::EPrimitiveType::TRIANGLE_FAN)
    {
      return FString::Printf(TEXT("Unsupported textured polygon primitiveType %d"), primitiveType);
    }
    const FString textureError = validateTextureSettings();
    if (!textureError.IsEmpty())
    {
      return textureError;
    }

    return "";
  }

  return FString::Printf(TEXT("Invalid symbol %d"), symbol);
}

bool USymbolConfig::IsLine() const
{
  // Circle
  if (GetSymbolType() == sbio::symbol::ESymbolType::CIRCLE)
  {
    return GetDrawingStyle() == sbio::symbol::EDrawingStyle::LINE;
  }

  // Polygon
  if (GetSymbolType() == sbio::symbol::ESymbolType::POLYGON)
  {
    const sbio::symbol::EPrimitiveType primitiveTypeValue = GetPrimitiveType();
    return primitiveTypeValue == sbio::symbol::EPrimitiveType::LINE || primitiveTypeValue == sbio::symbol::EPrimitiveType::LINE_STRIP || primitiveTypeValue == sbio::symbol::EPrimitiveType::LINE_LOOP;
  }

  // Textured circle
  if (GetSymbolType() == sbio::symbol::ESymbolType::TEXTURED_CIRCLE)
  {
     return false;
  }

  // Textured polygon
  if (GetSymbolType() == sbio::symbol::ESymbolType::TEXTURED_POLYGON)
  {
    return false;
  }

  return false;
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026