//Copyright SimBlocks LLC 2016-2026

#include "SlateCigiWidget.h"
#include "UnrealCigi_Plugin.h"
#include "UnrealCigi_PluginPrivate.h"
#include "UnrealCigiSymbolManager.h"
#include "unrealcigiEventHandler.h"
#include "Runtime\Launch\Resources\Version.h"
#include "SlateCustomVerts.h"
#include "unrealcigiEventHandler.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "unrealcigiUtil.h"
#include "SymbolLib/Symbol.h"
#include "SymbolLib/SymbolCircle.h"
#include "SymbolLib/SymbolPolygon.h"
#include "SymbolLib/SymbolText.h"
#include "SymbolLib/SymbolTexturedCircle.h"
#include "SymbolLib/SymbolTexturedPolygon.h"
#include "SymbolLib/SymbolSurfaceManager.h"

// If this is true, then debug geometry will be drawn on the screen
const bool SlateCigiDebug = false;

using namespace sbio;
using namespace sbio::symbol;
using namespace sbio::unrealcigi;

FColor ToUnrealColor(const sbio::SColor32& color)
{
  return FColor(color.r, color.g, color.b, color.a);
}

FVector2D ToUnrealVector2D(const sbio::math::Vec2f& vector)
{
  return FVector2D(vector.x(), vector.y());
}

UTexture2D* ResolveSymbolTexture(CUnrealCigiEventHandler* eventHandler, sbio::TextureID textureID, sbio::symbol::ETextureFilterMode filterMode, sbio::symbol::ETextureWrapMode wrapMode)
{
  // Get the texture from the event handler
  CUnrealCigiSymbolManager* symbolManager = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager.get();
  UTexture2D* texture = nullptr;
  if (symbolManager != nullptr)
  {
    texture = symbolManager->FindTexture(textureID);
  }
  if (!IsValid(texture))
  {
    return nullptr;
  }

  // Check if the texture settings need to be updated.
  bool settingsChanged = false;
  const TextureFilter desiredFilter = filterMode == sbio::symbol::ETextureFilterMode::NEAREST ? TF_Nearest : TF_Bilinear;
  if (texture->Filter != desiredFilter)
  {
    texture->Filter = desiredFilter;
    settingsChanged = true;
  }

  // Unreal Engine does not have a "mirror" wrap mode, so will treat it as "repeat".
  const TextureAddress desiredAddress = wrapMode == sbio::symbol::ETextureWrapMode::REPEAT ? TA_Wrap : TA_Clamp;
  if (texture->AddressX != desiredAddress || texture->AddressY != desiredAddress)
  {
    texture->AddressX = desiredAddress;
    texture->AddressY = desiredAddress;
    settingsChanged = true;
  }

  // If any of the texture settings changed, update the resource.
  if (settingsChanged)
  {
    texture->UpdateResourceWithParams(UTexture::EUpdateResourceFlags::None);
  }
  return texture;
}

DEFINE_LOG_CATEGORY(LogCigiSlateSymbols)

// ignore the VS suggestion about converting the macro to a const. Unreal requires this macro specifically.
#define LOCTEXT_NAMESPACE "SlateCigiWidget"

void SSlateCigiWidget::Construct(const FArguments& InArgs)
{
  SurfaceID = InArgs._SurfaceID;
}

int32 SSlateCigiWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
  // Get the position and size of the "window" that we are drawing in.

  // For VIEW surfaces:
  // Pos.X is the number of pixels from the left of the physical monitor screen to the left of the application window
  // Pos.Y is the number of pixels from the top of the physical monitor screen to the top of the application window
  FVector2D Pos = AllottedGeometry.GetAbsolutePosition();
  // For VIEW surfaces:
  // Size.X is the width of the application window, in pixels
  // Size.Y is the height of the application window, in pixels
  FVector2D Size = AllottedGeometry.GetAbsoluteSize();

  // The event handler should ALWAYS be valid, but it doesn't hurt to check
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (eventHandler == nullptr)
  {
    UE_LOG(LogCigiSlateSymbols, Warning, TEXT("SlateCigiWidget: EventHandler is nullptr!"));
    return LayerId;
  }
  // Similary, this class is only created for valid surfaces, so its surface ID should always be valid
  FUnrealSymbolSurface* surface = nullptr;
  if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager != nullptr)
  {
    surface = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSurface(SymbolSurfaceID(SurfaceID));
  }
  if (surface == nullptr)
  {
    UE_LOG(LogCigiSlateSymbols, Warning, TEXT("SlateCigiWidget: SurfaceFromID(%d) returned nullptr!"), SurfaceID);
    return LayerId;
  }

  // For VIEW surfaces only, this is used to ensure the aspect ratio does not distort symbol shape sizes
  FVector2D aspectRatio = FVector2D(1.0, 1.0);

  // For VIEW surfaces, extra steps are needed to calibrate the display
  if (surface->WidgetType == SymbolSurfaceType::VIEW)
  {
    // Modify the position and size of the window (in pixels) by the surface offset and size (percentages 0.0-1.0)
    Pos += Size * FVector2D(surface->Offset.X, surface->Offset.Y);
    Size *= surface->Size;
    if (Size.X <= KINDA_SMALL_NUMBER || Size.Y <= KINDA_SMALL_NUMBER)
    {
      return LayerId;
    }

    // Determine the aspect ratio of the window. The smaller size will always be 1.0.
    // Ex: If the window is 1,800 pixels wide and 1,200 pixels tall, then the ratio is (1.5,1.0)
    if (Size.X > Size.Y)
    {
      aspectRatio.X = Size.X / Size.Y;
    }
    else
    {
      aspectRatio.Y = Size.Y / Size.X;
    }
  }

  const FVector2D uvSize = surface->MaxUV - surface->MinUV;
  if (Size.X <= KINDA_SMALL_NUMBER || Size.Y <= KINDA_SMALL_NUMBER || uvSize.X <= KINDA_SMALL_NUMBER || uvSize.Y <= KINDA_SMALL_NUMBER)
  {
    return LayerId;
  }

  // Loop through each symbol in this surface and draw the symbol to the screen
  CUnrealCigiSymbolManager* symbolManager = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager.get();
  TArray<SymbolID> symbolIDs;
  if (symbolManager != nullptr)
  {
    symbolIDs = symbolManager->FindSymbolsOnSurface(surface->SurfaceID);
  }
  for (SymbolID symbolID : symbolIDs)
  {
    // Get the symbol information for each ID.
    sbio::symbol::CSymbol* symbol = nullptr;
    if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager != nullptr)
    {
      symbol = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSymbol(symbolID);
    }
    if (symbol == nullptr)
    {
      UE_LOG(LogCigiSlateSymbols, Warning, TEXT("SlateCigiWidget: Failed to display invalid SymbolID=%d"), symbolID.Value());
      continue;
    }
    // If the symbol is not visible, don't display it
    if (!symbol->GetEffectiveVisibility())
    {
      continue;
    }

    // Prepare the vertex grid, passing in the position and size of the window (updates every frame)
    SlateCustomVerts verts = SlateCustomVerts(Pos, Size);
    // Convert from UV coords to absolute screen space
    verts.SetUVGrid(surface->MinUV, surface->MaxUV);

    // Apply symbol transforms of this symbol and then all of its parent symbols (The ascending order is important!)
    sbio::symbol::CSymbol* currSymbol = symbol;
    TArray<SymbolID> usedParentIds = TArray<SymbolID>();
    // Do not allow the starting symbol to be used as a parent of itself
    usedParentIds.Add(symbol->GetSymbolID());
    while (currSymbol != nullptr)
    {
      // Build the list of CIGI symbol transforms
      verts.Transforms.Add(VertexTransform(ToUnrealVector2D(currSymbol->GetPosition()), currSymbol->GetRotation().Value(), ToUnrealVector2D(currSymbol->GetScale())));
      // If there is a circular parent reference, make sure we do not get stuck in an infinite loop
      SymbolID parentId = currSymbol->GetParentSymbolID();
      if (usedParentIds.Contains(parentId))
      {
        currSymbol = nullptr;
        break;
      }
      else
      {
        usedParentIds.Add(parentId);
        currSymbol = nullptr;
        if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager != nullptr)
        {
          currSymbol = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSymbol(parentId);
        }
      }
    }
    currSymbol = nullptr;

    // For VIEW surfaces only, revert the distortion from the window's aspect ratio
    if (surface->WidgetType == SymbolSurfaceType::VIEW && symbol->GetSymbolType() != ESymbolType::TEXT)
    {
      // Ex: If a window has width=800 pixels and height=1600 pixels, then aspectRatio=(1,2)
      // In this case, a circle (or any shape) will be stretched twice as tall since UV coords are a percentage of screen size
      // To prevent this, we scale the circle by 1/aspectRatio=(1,0.5). Twice as tall * half as tall = normal size.
      FVector2D invertAspectRatio = FVector2D(1.0, 1.0) / aspectRatio;
      // When creating this transform, specify applyParentScale=true so that this scale is applied on top of the symbol's scale.
      verts.Transforms.Add(VertexTransform(FVector2D(0, 0), 0, invertAspectRatio, true));
    }

    // Display the symbol based on its type
    if (symbol->GetSymbolType() == ESymbolType::CIRCLE)
    {
      PaintCircle(symbol, verts);
    }
    else if (symbol->GetSymbolType() == ESymbolType::POLYGON)
    {
      PaintPolygon(symbol, verts);
    }
    else if (symbol->GetSymbolType() == ESymbolType::TEXT)
    {
      // Text symbols use FSlateDrawElement::MakeText (in the PaintText function) instead of using vertices.
      PaintText(symbol, symbolID, surface, AllottedGeometry, OutDrawElements, LayerId);
      // Reset the SlateCustomVerts class to make extra sure that there are no vertices being displayed.
      verts = SlateCustomVerts(Pos, Size);
    }
    else if (symbol->GetSymbolType() == ESymbolType::TEXTURED_CIRCLE)
    {
      PaintTexturedCircle(symbol, verts);
    }
    else if (symbol->GetSymbolType() == ESymbolType::TEXTURED_POLYGON)
    {
      PaintTexturedPolygon(symbol, verts);
    }
    // If the symbol is just a generic Symbol, don't display anything
    else
    {
      UE_LOG(LogCigiSlateSymbols, Log, TEXT("SlateCigiWidget: Found generic Symbol with id=%d, nothing to display"), symbolID.Value());
    }

    // Draw this symbol's vertices (if any)
    if (verts.Vertices.Num() > 0)
    {
      // Set up a default slate brush
      FSlateBrush SlateBrush;

      // If the symbol has a texture, set it up for the brush.
      // Otherwise, the brush will be blank and the color of the vertices will be used.
      if (IsValid(verts.Texture))
      {
        SlateBrush.SetResourceObject(verts.Texture);
        SlateBrush.ImageSize = FVector2D(verts.Texture->GetSizeX(), verts.Texture->GetSizeY());
      }

      const FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(SlateBrush);
      // Use the vertices and indices from "verts". The SlateCustomVerts class handles their ordering and coordinate conversions.
      FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, verts.Vertices, verts.Indices, nullptr, 0, 0);
    }
  }

  // Note: This can be very useful for debugging issues, so the code will not be deleted even though it is almost never used.
  if (SlateCigiDebug)
  {
    // Debug: Display a randomly-colored quad to show the bounds of this surface
    SlateCustomVerts verts = SlateCustomVerts(Pos, Size);
    verts.SetUVGrid(surface->MinUV, surface->MaxUV);
    verts.AddQuad(FColor::Orange.WithAlpha(100), surface->MinUV, FVector2D(surface->MinUV.X, surface->MaxUV.Y), surface->MaxUV, FVector2D(surface->MaxUV.X, surface->MinUV.Y));
    const FSlateBrush SlateBrush = FSlateBrush();
    const FSlateResourceHandle Handle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(SlateBrush);
    FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, Handle, verts.Vertices, verts.Indices, nullptr, 0, 0);
    // Debug Text
    FText displayText = FText::FromString("Debug Surface");
    FSlateFontInfo fontInfo = FSlateFontInfo(GEngine->GetSmallFont(), 25);
    FPaintGeometry paintGeometry = AllottedGeometry.ToPaintGeometry();
    FSlateDrawElement::MakeText(OutDrawElements, LayerId, paintGeometry, displayText, fontInfo);
  }

  return LayerId;
}

void SSlateCigiWidget::PaintCircle(sbio::symbol::CSymbol* symbol, SlateCustomVerts& verts) const
{
  // Check if symbol is valid.
  if (symbol == nullptr)
  {
    return;
  }

  // Get the geometry cache for this symbol. The cache contains the triangle vertices that define the circle shape.
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  CUnrealCigiSymbolManager* symbolManager = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager.get();
  const FSymbolGeometryCache* cache = symbolManager == nullptr ? nullptr : symbolManager->GetCircleGeometry(symbol->GetSymbolID());
  if (cache == nullptr)
  {
    return;
  }

  const FColor color = ToUnrealColor(symbol->GetColor());

  // Add each triangle from the cache to the SlateCustomVerts object. Each triangle is defined by three vertices.
  for (int32 i = 0; i + 2 < cache->TriangleVertices.Num(); i += 3)
  {
    verts.AddTriangle(color, cache->TriangleVertices[i], cache->TriangleVertices[i + 1], cache->TriangleVertices[i + 2]);
  }
}

void SSlateCigiWidget::PaintPolygon(sbio::symbol::CSymbol* symbol, SlateCustomVerts& verts) const
{
  // Check if symbol is valid.
  if (symbol == nullptr)
  {
    return;
  }

  // Get the geometry cache for this symbol. The cache contains the triangle vertices that define the polygon shape.
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  CUnrealCigiSymbolManager* symbolManager = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager.get();
  const FSymbolGeometryCache* cache = symbolManager == nullptr ? nullptr : symbolManager->GetPolygonGeometry(symbol->GetSymbolID());
  if (cache == nullptr)
  {
    return;
  }

  const FColor color = ToUnrealColor(symbol->GetColor());

  // Add each triangle from the cache to the SlateCustomVerts object. Each triangle is defined by three vertices.
  for (int32 i = 0; i + 2 < cache->TriangleVertices.Num(); i += 3)
  {
    verts.AddTriangle(color, cache->TriangleVertices[i], cache->TriangleVertices[i + 1], cache->TriangleVertices[i + 2]);
  }
}

void SSlateCigiWidget::PaintTexturedCircle(sbio::symbol::CSymbol* symbol, SlateCustomVerts& verts) const
{
  // Check if symbol is valid.
  if (symbol == nullptr)
  {
    return;
  }

  // Get the event handler to access the geometry cache and textures
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (eventHandler == nullptr)
  {
    return;
  }

  // Get the geometry cache for this symbol. The cache contains the triangle vertices that define the textured circle shape.
  CUnrealCigiSymbolManager* symbolManager = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager.get();
  const FTexturedCircleGeometryCache* cache = symbolManager == nullptr ? nullptr : symbolManager->GetTexturedCircleGeometry(symbol->GetSymbolID());
  if (cache == nullptr)
  {
    return;
  }

  UTexture2D* texture = ResolveSymbolTexture(eventHandler, cache->TextureID, cache->FilterMode, cache->WrapMode);

  // If the texture is not valid, log a warning and return without drawing anything.
  if (!IsValid(texture))
  {
    UE_LOG(LogCigiSlateSymbols, Warning, TEXT("SlateCigiWidget: No Unreal texture is registered for TextureID=%d"), cache->TextureID.Value());
    return;
  }

  verts.Texture = texture;
  const FColor color = ToUnrealColor(symbol->GetColor());

  // Add each triangle from the cache to the SlateCustomVerts object. Each triangle is defined by three vertices.
  for (int32 vertexIndex = 0; vertexIndex + 2 < cache->TriangleVertices.Num(); vertexIndex += 3)
  {
    const FTexturedSymbolVertex& a = cache->TriangleVertices[vertexIndex];
    const FTexturedSymbolVertex& b = cache->TriangleVertices[vertexIndex + 1];
    const FTexturedSymbolVertex& c = cache->TriangleVertices[vertexIndex + 2];
    verts.AddTexturedTriangle(color, a.Position, a.TextureST, b.Position, b.TextureST, c.Position, c.TextureST);
  }
}

void SSlateCigiWidget::PaintTexturedPolygon(sbio::symbol::CSymbol* symbol, SlateCustomVerts& verts) const
{
  // Check if symbol is valid.
  if (symbol == nullptr)
  {
    return;
  }

  // Get the geometry cache for this symbol. The cache contains the triangle vertices that define the textured polygon shape.
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  CUnrealCigiSymbolManager* symbolManager = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager.get();
  const FTexturedPolygonGeometryCache* cache = symbolManager == nullptr ? nullptr : symbolManager->GetTexturedPolygonGeometry(symbol->GetSymbolID());
  if (cache == nullptr)
  {
    return;
  }

  // Get the Unreal texture for this symbol's texture ID, filter mode, and wrap mode.
  UTexture2D* texture = ResolveSymbolTexture(eventHandler, cache->TextureID, cache->FilterMode, cache->WrapMode);

  // If the texture is not valid, log a warning and return without drawing anything.
  if (!IsValid(texture))
  {
    UE_LOG(LogCigiSlateSymbols, Warning, TEXT("SlateCigiWidget: No Unreal texture is registered for TextureID=%d"), cache->TextureID.Value());
    return;
  }

  verts.Texture = texture;
  const FColor color = ToUnrealColor(symbol->GetColor());

  // Add each triangle from the cache to the SlateCustomVerts object. Each triangle is defined by three vertices.
  for (int32 i = 0; i + 2 < cache->TriangleVertices.Num(); i += 3)
  {
    const FTexturedSymbolVertex& vertexA = cache->TriangleVertices[i];
    const FTexturedSymbolVertex& vertexB = cache->TriangleVertices[i + 1];
    const FTexturedSymbolVertex& vertexC = cache->TriangleVertices[i + 2];
    verts.AddTexturedTriangle(color, vertexA.Position, vertexA.TextureST, vertexB.Position, vertexB.TextureST, vertexC.Position, vertexC.TextureST);
  }
}

void SSlateCigiWidget::PaintText(sbio::symbol::CSymbol* symbol, SymbolID symbolID, FUnrealSymbolSurface* surface, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const
{
  // If the symbol or surface is invalid, do not attempt to display it
  if (symbol == nullptr || surface == nullptr)
  {
    return;
  }

  // The event handler should ALWAYS be valid, but it doesn't hurt to check
  CUnrealCigiEventHandler* eventHandler = FUnrealCigi_PluginModule::globals.pEventHandler.get();
  if (eventHandler == nullptr)
  {
    return;
  }

  // Get the text cache for this symbol. The cache contains the display string, font ID, font size, alignment, and measured size.
  CUnrealCigiSymbolManager* symbolManager = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager.get();
  const FSymbolTextCache* cache = nullptr;
  if (symbolManager != nullptr)
  {
    cache = symbolManager->GetText(symbolID);
  }
  if (cache == nullptr)
  {
    return;
  }

  FText displayText = FText::FromString(cache->DisplayString);

  // Get the font object from the event handler using the font ID from the cache.
  UFont* fontObject = nullptr;
  if (symbolManager != nullptr)
  {
    fontObject = symbolManager->FindFont(cache->FontID);
  }
  if (fontObject == nullptr)
  {
    if (GEngine != nullptr)
    {
      fontObject = GEngine->GetSmallFont();
    }
    if (fontObject == nullptr)
    {
      return;
    }
  }

  float fontSize = cache->fFontSize;

  // If the font size is not set, use a default size of 10
  if (fontSize <= 0)
  {
    fontSize = 10;
  }

  FSlateFontInfo fontInfo = FSlateFontInfo(fontObject, fontSize);

  // Calculate the alignment and orientation of the text (in absolute units)
  const FVector2D textSize = cache->bMeasurementValid ? cache->MeasuredSize : FVector2D::ZeroVector;
  FVector2D textRefOffset = FVector2D(0, 0);
  // The text reference point in Unreal is the top-left corner
  // So this draws the text up and left so that the reference point is at the center, bottom-right, etc
  switch (cache->Alignment)
  {
  case sbio::symbol::ETextAlignment::TOP_LEFT:
  case sbio::symbol::ETextAlignment::CENTER_LEFT:
  case sbio::symbol::ETextAlignment::BOTTOM_LEFT:
    textRefOffset.X = 0;
    break;
  case sbio::symbol::ETextAlignment::TOP_CENTER:
  case sbio::symbol::ETextAlignment::CENTER:
  case sbio::symbol::ETextAlignment::BOTTOM_CENTER:
    textRefOffset.X = -textSize.X / 2.0f;
    break;
  case sbio::symbol::ETextAlignment::TOP_RIGHT:
  case sbio::symbol::ETextAlignment::CENTER_RIGHT:
  case sbio::symbol::ETextAlignment::BOTTOM_RIGHT:
    textRefOffset.X = -textSize.X;
    break;
  }
  switch (cache->Alignment)
  {
  case sbio::symbol::ETextAlignment::TOP_LEFT:
  case sbio::symbol::ETextAlignment::TOP_CENTER:
  case sbio::symbol::ETextAlignment::TOP_RIGHT:
    textRefOffset.Y = 0;
    break;
  case sbio::symbol::ETextAlignment::CENTER_LEFT:
  case sbio::symbol::ETextAlignment::CENTER:
  case sbio::symbol::ETextAlignment::CENTER_RIGHT:
    textRefOffset.Y = -textSize.Y / 2.0f;
    break;
  case sbio::symbol::ETextAlignment::BOTTOM_LEFT:
  case sbio::symbol::ETextAlignment::BOTTOM_CENTER:
  case sbio::symbol::ETextAlignment::BOTTOM_RIGHT:
    textRefOffset.Y = -textSize.Y;
    break;
  }

  // Create the first render transform.
  // This moves the text up and left to change from Unreal's top-left alignment to the CIGI-specified alignment
  FSlateRenderTransform textRender = FSlateRenderTransform(textRefOffset);
  UE_LOG(LogCigiSlateSymbols, SLATE_SYMBOLS, TEXT("SlateCigiWidget: Text id=%d, textRefOffset=(%.2f,%.2f), textRender=(%.2f,%.2f)"), symbolID.Value(), textRefOffset.X, textRefOffset.Y, textRender.GetTranslation().X, textRender.GetTranslation().Y);

  // Apply the text symbol's own scale before parent transforms.
  // Parent symbol scales are applied while traversing the parent chain below.
  FVector2D textScale = ToUnrealVector2D(symbol->GetScale());
  // Apply the symbol scale to the render transform
  textRender = textRender.Concatenate(FSlateRenderTransform(FScale2D(textScale.X, textScale.Y)));
  UE_LOG(LogCigiSlateSymbols, SLATE_SYMBOLS, TEXT("SlateCigiWidget: Text id=%d, symbolScale=(%.2f,%.2f), textScale=(%.2f,%.2f), textRender=(%.2f,%.2f)"), symbolID.Value(), textScale.X, textScale.Y, textScale.X, textScale.Y, textRender.GetTranslation().X, textRender.GetTranslation().Y);

  // Text positions are based on the local size (as opposed to shape vertices that use absolute size)
  // The top-left corner of the app window is (0,0) (as opposed to shape vertices that have absolute pos as the top-left corner)
  FVector2D agLocalSize = AllottedGeometry.GetLocalSize();

  // Determine the conversion from UV coords to Local coords (all of these operations are element-wise)
  FVector2D uvSize = surface->MaxUV - surface->MinUV;

  // If the UV size is zero, then the surface is invalid and cannot be displayed.
  if (FMath::IsNearlyZero(uvSize.X) || FMath::IsNearlyZero(uvSize.Y))
  {
    UE_LOG(LogCigiSlateSymbols, Warning, TEXT("SlateCigiWidget: Text id=%d has an invalid surface UV range (%.2f,%.2f)"), symbolID.Value(), uvSize.X, uvSize.Y);
    return;
  }

  // The UV-to-Local offset is the difference between the top-left corner of the surface's UV range and the top-left corner of the local size.
  FVector2D uvToLocalOffset = FVector2D(0, 0) - surface->MinUV;
  FVector2D uvToLocalScale = agLocalSize / uvSize;

  // Apply the UV-to-Local scale to the render transform
  textRender = textRender.Concatenate(FSlateRenderTransform(FScale2D(uvToLocalScale.X, uvToLocalScale.Y)));

  // Apply the rotation and offset of this symbol and all of its parents
  // When using Concatenate, the left-hand transform is applied first, then the parameter is applied second
  // This means that each transform is modifying all of the transforms before it. Later concatenate = parent / higher-level transform.
  sbio::symbol::CSymbol* currSymbol = symbol;
  TArray<SymbolID> usedParentIds = TArray<SymbolID>();
  // Do not allow the starting symbol to be used as a parent of itself
  usedParentIds.Add(symbol->GetSymbolID());
  while (currSymbol != nullptr)
  {
    // Get the rotation of the symbol in degrees. Convert from CIGI counterclockwise to Unreal clockwise rotation.
    Degrees rotDegrees = Degrees(360.0f) - currSymbol->GetRotation();
    FQuat2D rotQuat = FQuat2D(sbio::math::DegreesToRadians(rotDegrees).Value());
    // Apply the rotation of this symbol to the text render
    textRender = textRender.Concatenate(FSlateRenderTransform(rotQuat));
    UE_LOG(LogCigiSlateSymbols, SLATE_SYMBOLS, TEXT("SlateCigiWidget: Text id=%d, ROTATE symbol id=%d: symbolRotation=%.2f, rotDegrees=%.2f, textRender=(%.2f,%.2f)"), symbolID.Value(), currSymbol->GetSymbolID().Value(), currSymbol->GetRotation().Value(), rotDegrees.Value(),
           textRender.GetTranslation().X, textRender.GetTranslation().Y);

    // Convert the CIGI UV position into an Unreal Local position on the screen
    const FVector2D symbolPosition = ToUnrealVector2D(currSymbol->GetPosition());
    FVector2D localPos = (symbolPosition + uvToLocalOffset) * uvToLocalScale;
    // Flip the Y-axis to convert from CIGI's "Y+ is up" to Unreal's "Y+ is down".
    localPos.Y = agLocalSize.Y - localPos.Y;

    // When attached to parent symbols, treat the parent location like the center of the screen instead of the top-left corner
    if (currSymbol != symbol)
    {
      FVector2D oldLocalPos = localPos;
      localPos = localPos - (agLocalSize * 0.5);
      UE_LOG(LogCigiSlateSymbols, SLATE_SYMBOLS, TEXT("SlateCigiWidget: Text id=%d, ADJUST symbol id=%d: localPos(%.2f,%.2f) = localPos(%.2f,%.2f) - agLocalSize*0.5(%.2f,%.2f)"), symbolID.Value(), currSymbol->GetSymbolID().Value(), localPos.X, localPos.Y, oldLocalPos.X, oldLocalPos.Y,
             (agLocalSize * 0.5).X, (agLocalSize * 0.5).Y);
    }

    // Apply the position offset of this symbol to the text render
    textRender = textRender.Concatenate(FSlateRenderTransform(localPos));
    UE_LOG(LogCigiSlateSymbols, SLATE_SYMBOLS, TEXT("SlateCigiWidget: Text id=%d, OFFSET symbol id=%d: symbolOffset=(%.2f,%.2f), screenPos=(%.2f,%.2f), textRender=(%.2f,%.2f)"), symbolID.Value(), currSymbol->GetSymbolID().Value(), symbolPosition.X, symbolPosition.Y, localPos.X, localPos.Y,
           textRender.GetTranslation().X, textRender.GetTranslation().Y);

    // Resolve the parent and apply its scale to all previously accumulated child transforms.
    SymbolID parentId = currSymbol->GetParentSymbolID();
    if (usedParentIds.Contains(parentId))
    {
      currSymbol = nullptr;
      break;
    }
    else
    {
      usedParentIds.Add(parentId);
      sbio::symbol::CSymbol* parentSymbol = nullptr;
      if (FUnrealCigi_PluginModule::globals.pUnrealSymbolManager != nullptr)
      {
        parentSymbol = FUnrealCigi_PluginModule::globals.pUnrealSymbolManager->FindSymbol(parentId);
      }
      if (parentSymbol == nullptr)
      {
        currSymbol = nullptr;
        break;
      }

      const sbio::math::Vec2f parentScale = parentSymbol->GetScale();
      const float parentScaleU = parentScale.x();
      const float parentScaleV = parentScale.y();
      textRender = textRender.Concatenate(FSlateRenderTransform(FScale2D(parentScaleU, parentScaleV)));

      currSymbol = parentSymbol;
    }
  }

  // Defaults for AllottedGeometry.ToPaintGeometry.
  FSlateLayoutTransform textLayout = FSlateLayoutTransform();
  FVector2D rtPivot = FVector2D(0, 0);

  // Debug Info
  FVector2D trTrans = textRender.GetTranslation();
  FVector2D tlTrans = textLayout.GetTranslation();
  float tlScale = textLayout.GetScale();
  UE_LOG(LogCigiSlateSymbols, SLATE_SYMBOLS, TEXT("SlateCigiWidget: Text id=%d, ToPaintGeometry({%.2f,%.2f} | {x=%.2f,y=%.2f,s=%.2f} | {%.2f,%.2f} | {%.2f.%.2f})"), symbolID.Value(), agLocalSize.X, agLocalSize.Y, /*|*/ tlTrans.X, tlTrans.Y, tlScale, /*|*/ trTrans.X, trTrans.Y, /*|*/ rtPivot.X,
         rtPivot.Y);

  // Create the paint geometry for this text
  FPaintGeometry paintGeometry = AllottedGeometry.ToPaintGeometry(agLocalSize, textLayout, textRender, rtPivot);

  UE_LOG(LogCigiSlateSymbols, SLATE_SYMBOLS, TEXT("SlateCigiWidget: Text with id %d has size (%.2f,%.2f) and refOffset (%.2f,%.2f)"), symbolID.Value(), textSize.X, textSize.Y, textRefOffset.X, textRefOffset.Y);

  // Test is displayed using its own FSlateDrawElement function, separate from how vertices are normally displayed.
  FSlateDrawElement::MakeText(OutDrawElements, LayerId, paintGeometry, displayText, fontInfo, ESlateDrawEffect::None, FLinearColor(ToUnrealColor(symbol->GetColor())));
}

#undef LOCTEXT_NAMESPACE

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026