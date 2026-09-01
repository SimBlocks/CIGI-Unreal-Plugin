//Copyright SimBlocks LLC 2016-2026
#include "UnrealCigiSymbolManager.h"
#include "SlateCustomVerts.h"
#include "SymbolConfig.h"
#include "UnrealCigiUtil.h"
#include "Engine/Font.h"
#include "Framework/Application/SlateApplication.h"
#include "SymbolLib/SymbolCircle.h"
#include "SymbolLib/SymbolPolygon.h"
#include "SymbolLib/SymbolText.h"
#include "SymbolLib/SymbolTexturedCircle.h"
#include "SymbolLib/SymbolTexturedPolygon.h"

namespace sbio
{
  namespace unrealcigi
  {
    bool CUnrealCigiSymbolManager::RegisterSymbolTemplate(int32 templateID, USymbolConfig* symbolTemplate)
    {
      if (!IsValid(symbolTemplate) || symbolTemplate->IsRooted() || SymbolTemplates.Contains(templateID))
      {
        return false;
      }
      symbolTemplate->AddToRoot();
      SymbolTemplates.Emplace(templateID, symbolTemplate);
      return true;
    }

    USymbolConfig* CUnrealCigiSymbolManager::FindSymbolTemplate(int32 templateID) const
    {
      // Attempt to find the symbol template in the map
      USymbolConfig* const* symbolTemplate = SymbolTemplates.Find(templateID);
      
      // If the symbol template is not found, return nullptr
      if (symbolTemplate == nullptr)
      {
        return nullptr;
      }

      return *symbolTemplate;
    }

    void CUnrealCigiSymbolManager::ClearSymbolTemplates()
    {
      for (TPair<int32, USymbolConfig*>& pair : SymbolTemplates)
      {
        if (IsValid(pair.Value) && pair.Value->IsRooted())
        {
          pair.Value->RemoveFromRoot();
        }
      }
      SymbolTemplates.Empty();
    }

    CUnrealCigiSymbolManager::CUnrealCigiSymbolManager(const std::shared_ptr<sbio::symbol::CSymbolSurfaceManager>& symbolSurfaceManager) : SymbolSurfaceManager(symbolSurfaceManager)
    {
    }

    CUnrealCigiSymbolManager::~CUnrealCigiSymbolManager()
    {
      ClearSymbolTemplates();
    }

    sbio::symbol::CSymbol* CUnrealCigiSymbolManager::FindSymbol(sbio::symbol::SymbolID symbolID) const
    {
      // Check if the symbol surface manager is valid
      if (SymbolSurfaceManager == nullptr)
      {
        return nullptr;
      }

      // Attempt to find the symbol in the symbol surface manager
      return SymbolSurfaceManager->GetSymbol(symbolID);
    }

    sbio::symbol::CSymbol* CUnrealCigiSymbolManager::FindSymbol(sbio::symbol::SymbolID symbolID, sbio::symbol::ESymbolType symbolType) const
    {
      sbio::symbol::CSymbol* symbol = FindSymbol(symbolID);

      // Check if the symbol is found and matches the requested type
      if (symbol == nullptr || symbol->GetSymbolType() != symbolType)
      {
        return nullptr;
      }

      // If the symbol is found and matches the requested type, return it
      return symbol;
    }

    sbio::symbol::CSymbol* CUnrealCigiSymbolManager::CreateSymbol(sbio::symbol::SymbolID symbolID, sbio::symbol::ESymbolType symbolType)
    {
      // Attempt to find the symbol in the symbol surface manager
      sbio::symbol::CSymbol* symbol = FindSymbol(symbolID, symbolType);

      // If the symbol already exists, return it
      if (symbol != nullptr)
      {
        return symbol;
      }

      // Check if the symbol surface manager is valid and if the symbol already exists
      if (SymbolSurfaceManager == nullptr || SymbolSurfaceManager->GetSymbol(symbolID) != nullptr)
      {
        return nullptr;
      }

      // Create a new symbol and add it to the symbol surface manager
      SymbolSurfaceManager->AddSymbol(symbolID, std::make_unique<sbio::cigi::ig::CCigiSymbol>(symbolID, symbolType));

      // After adding the symbol, attempt to find it again to return the pointer
      return FindSymbol(symbolID, symbolType);
    }

    void CUnrealCigiSymbolManager::RemoveSymbol(sbio::symbol::SymbolID symbolID)
    {
      RemoveRenderCaches(symbolID);
      if (SymbolSurfaceManager != nullptr)
      {
        SymbolSurfaceManager->RemoveSymbol(symbolID);
      }
    }

    FUnrealSymbolSurface* CUnrealCigiSymbolManager::FindSurface(sbio::symbol::SymbolSurfaceID surfaceID)
    {
      return UnrealSurfaces.Find(surfaceID.Value());
    }

    FUnrealSymbolSurface& CUnrealCigiSymbolManager::CreateSurface(sbio::symbol::SymbolSurfaceID surfaceID)
    {
      // Check if the surface already exists in the UnrealSurfaces map
      FUnrealSymbolSurface* surface = UnrealSurfaces.Find(surfaceID.Value());

      // If the surface does not exist, create a new one and add it to the UnrealSurfaces map
      if (surface == nullptr)
      {
        return UnrealSurfaces.Add(surfaceID.Value(), FUnrealSymbolSurface(surfaceID));
      }

      return *surface;
    }

    FUnrealSymbolSurface* CUnrealCigiSymbolManager::UpdateBillboardSurface(const sbio::ig::symbol::SUpdateEntityBillboardSymbolSurfaceMessage& data)
    {
      // Find the surface in the UnrealSurfaces map using the provided SurfaceID
      FUnrealSymbolSurface* surface = FindSurface(data.SurfaceID);

      // If the surface is found, update its properties based on the provided data
      if (surface != nullptr)
      {
        surface->Enabled = true;
        surface->Type = SymbolSurfaceType::BILLBOARD;
        surface->MinUV = FVector2D(data.uvMin.U, data.uvMin.V);
        surface->MaxUV = FVector2D(data.uvMax.U, data.uvMax.V);
        surface->Offset = sbio::unrealcigi::utils::BodyCoordinatesToFVector(data.Offset);
        surface->Size = FVector2D(data.Width, data.Height);
      }

      return surface;
    }

    FUnrealSymbolSurface* CUnrealCigiSymbolManager::UpdateWorldSurface(const sbio::ig::symbol::SUpdateSymbolSurfaceMessage& data)
    {
      // Find the surface in the UnrealSurfaces map using the provided SurfaceID
      FUnrealSymbolSurface* surface = FindSurface(data.SurfaceID);

      // If the surface is found, update its properties based on the provided data
      if (surface != nullptr)
      {
        surface->Enabled = true;
        surface->Type = SymbolSurfaceType::WORLD;
        surface->MinUV = FVector2D(data.uvMin.U, data.uvMin.V);
        surface->MaxUV = FVector2D(data.uvMax.U, data.uvMax.V);
        surface->Offset = sbio::unrealcigi::utils::BodyCoordinatesToFVector(data.Offset);
        surface->Size = FVector2D(data.Width, data.Height);
        surface->Rotation = FQuat::MakeFromEuler(FVector(data.Roll, data.Pitch, data.Yaw));
      }

      return surface;
    }

    FUnrealSymbolSurface* CUnrealCigiSymbolManager::UpdateViewSurface(const sbio::ig::symbol::SUpdateViewSymbolSurfaceMessage& data)
    {
      // Find the surface in the UnrealSurfaces map using the provided SurfaceID
      FUnrealSymbolSurface* surface = FindSurface(data.SurfaceID);

      // If the surface is found, update its properties based on the provided data
      if (surface != nullptr)
      {
        const float left = FMath::Clamp(data.Left, 0.0f, 1.0f);
        const float right = FMath::Clamp(data.Right, 0.0f, 1.0f);
        const float top = FMath::Clamp(data.Top, 0.0f, 1.0f);
        const float bottom = FMath::Clamp(data.Bottom, 0.0f, 1.0f);
        surface->Enabled = true;
        surface->Type = SymbolSurfaceType::VIEW;
        surface->MinUV = FVector2D(data.uvMin.U, data.uvMin.V);
        surface->MaxUV = FVector2D(data.uvMax.U, data.uvMax.V);
        surface->Offset = FVector(left, 1 - top, 0);
        surface->Size = FVector2D(right - left, top - bottom);
      }
      return surface;
    }

    void CUnrealCigiSymbolManager::RemoveSurface(sbio::symbol::SymbolSurfaceID surfaceID)
    {
      if (SymbolSurfaceManager != nullptr)
      {
        // Iterate through all symbols in the symbol surface manager and reset the surface ID for any symbols that are associated with the specified surface ID.
        for (const auto& symbolPair : SymbolSurfaceManager->GetSymbols())
        {
          if (symbolPair.second != nullptr && symbolPair.second->GetSymbolSurfaceID() == surfaceID)
          {
            symbolPair.second->SetSymbolSurfaceID(sbio::symbol::UnknownSymbolSurfaceID);
          }
        }

        // Remove the surface from the symbol surface manager
        SymbolSurfaceManager->RemoveSymbolSurface(surfaceID);
      }

      // Remove the surface from the UnrealSurfaces map
      UnrealSurfaces.Remove(surfaceID.Value());
    }

    TArray<sbio::symbol::SymbolID> CUnrealCigiSymbolManager::FindSymbolsOnSurface(sbio::symbol::SymbolSurfaceID surfaceID) const
    {
      TArray<sbio::symbol::SymbolID> symbolIDs;

      // Check if the symbol surface manager is valid
      if (SymbolSurfaceManager == nullptr)
      {
        return symbolIDs;
      }

      // Iterate through all symbols in the symbol surface manager and collect the IDs of symbols that are associated with the specified surface ID.
      for (const auto& symbolPair : SymbolSurfaceManager->GetSymbols())
      {
        if (symbolPair.second != nullptr && symbolPair.second->GetSymbolSurfaceID() == surfaceID)
        {
          symbolIDs.Add(symbolPair.first);
        }
      }
      return symbolIDs;
    }

    bool CUnrealCigiSymbolManager::RegisterTexture(sbio::TextureID textureID, UTexture2D* texture)
    {
      if (textureID == sbio::UnknownTextureID || !IsValid(texture))
      {
        return false;
      }
      Textures.Add(textureID.Value(), TStrongObjectPtr<UTexture2D>(texture));
      return true;
    }

    UTexture2D* CUnrealCigiSymbolManager::FindTexture(sbio::TextureID textureID) const
    {
      // Attempt to find the texture in the map
      const TStrongObjectPtr<UTexture2D>* texture = Textures.Find(textureID.Value());

      // If the texture is not found, return nullptr
      if (texture == nullptr)
      {
        return nullptr;
      }

      return texture->Get();
    }

    void CUnrealCigiSymbolManager::ClearTextures()
    {
      Textures.Empty();
    }

    void CUnrealCigiSymbolManager::ClearSurfaces()
    {
      UnrealSurfaces.Empty();
    }

    void CUnrealCigiSymbolManager::InvalidateTexturedCircleGeometry(sbio::symbol::SymbolID symbolID)
    {
      // Increment the geometry revision for the given symbol ID to indicate that the cached geometry is no longer valid.
      FTexturedCircleGeometryCache& cache = TexturedCircleGeometryCaches.FindOrAdd(symbolID.Value());
      ++cache.nGeometryRevision;

      // If the geometry revision is zero, set it to one and reset the cached geometry revision to zero.
      if (cache.nGeometryRevision == 0)
      {
        cache.nGeometryRevision = 1;
        cache.nCachedGeometryRevision = 0;
      }
    }

    const FTexturedCircleGeometryCache* CUnrealCigiSymbolManager::GetTexturedCircleGeometry(sbio::symbol::SymbolID symbolID)
    {
      // Retrieve the cached geometry for the given symbol ID. If it does not exist, create a new cache entry.
      FTexturedCircleGeometryCache& cache = TexturedCircleGeometryCaches.FindOrAdd(symbolID.Value());
      if (cache.nCachedGeometryRevision == cache.nGeometryRevision)
      {
        return &cache;
      }

      // If the cached geometry revision is not equal to the current geometry revision, the cached geometry is outdated and needs to be regenerated.
      const sbio::symbol::CSymbolTexturedCircle* circle = FindGeometry<sbio::symbol::CSymbolTexturedCircle>(symbolID, sbio::symbol::ESymbolType::TEXTURED_CIRCLE);
      if (circle == nullptr)
      {
        return nullptr;
      }

      // Retrieve the properties of the textured circle and update the cache
      const sbio::symbol::SSymbolTexturedCircle& properties = circle->GetProperties();
      cache.TriangleVertices.Reset();
      cache.TextureID = properties.textureID;
      cache.FilterMode = properties.eTextureFilter;
      cache.WrapMode = properties.eTextureWrap;

      const float segmentDegrees = 10.0f;

      // Iterate through each textured circle element in the properties and generate the triangle vertices for rendering.
      for (const sbio::symbol::STexturedCircleProperties& element : properties.circles)
      {
        float outerRadius = FMath::Max(0.0f, element.fRadius);
        float innerRadius = FMath::Max(0.0f, element.fInnerRadius);

        // Swap the inner and outer radii if the inner radius is greater than the outer radius
        // to ensure that the inner radius is always less than or equal to the outer radius.
        if (innerRadius > outerRadius)
        {
          Swap(innerRadius, outerRadius);
        }

        // Skip the element if the outer radius is nearly zero or if the inner and outer radii are nearly equal.
        if (FMath::IsNearlyZero(outerRadius) || FMath::IsNearlyEqual(innerRadius, outerRadius))
        {
          continue;
        }

        // Normalize the start and end angles to the range [0, 360) degrees and ensure that the end angle is greater than the start angle.
        float startAngle = FMath::Fmod(element.startAngle.Value(), 360.0f);
        float endAngle = FMath::Fmod(element.endAngle.Value(), 360.0f);
        if (startAngle < 0.0f)
        {
          startAngle += 360.0f;
        }
        if (endAngle < 0.0f)
        {
          endAngle += 360.0f;
        }

        // If the end angle is less than or equal to the start angle, add 360 degrees to the end angle to ensure that the segment is drawn in the correct direction.
        if (endAngle <= startAngle)
        {
          endAngle += 360.0f;
        }

        // Convert the center UV and texture ST coordinates from sbio::math::Vec2f to FVector2D for use in Unreal Engine.
        const FVector2D center(element.centerUV.x(), element.centerUV.y());
        const FVector2D centerST(element.centerTextureST.x(), element.centerTextureST.y());

        // Define a lambda function to calculate the position of a point on the circle for a given angle and radius.
        const auto circlePosition = [&center](float angle, float radius)
        {
          const float radians = FMath::DegreesToRadians(angle);
          return center + FVector2D(FMath::Cos(radians), FMath::Sin(radians)) * radius;
        };

        // Define a lambda function to calculate the texture coordinates for a given angle and radius, taking into account the texture map rotation and scaling.
        const auto texturePosition = [&centerST, outerRadius, &element](float angle, float radius)
        {
          const float radians = FMath::DegreesToRadians(angle + element.fTextureMapRotation);
          const float mappedRadius = element.fTextureMapRadius * radius / outerRadius;
          return centerST + FVector2D(FMath::Cos(radians), FMath::Sin(radians)) * mappedRadius;
        };

        // Define a lambda function to add a vertex to the triangle vertices cache.
        const auto addVertex = [&cache](const FVector2D& position, const FVector2D& textureST)
        {
          FTexturedSymbolVertex vertex;
          vertex.Position = position;
          vertex.TextureST = textureST;
          cache.TriangleVertices.Add(vertex);
        };

        // Generate the triangle vertices for the textured circle segment by segment.
        for (float angle = startAngle; angle < endAngle - KINDA_SMALL_NUMBER; angle += segmentDegrees)
        {
          // Calculate the next angle for the segment, ensuring it does not exceed the end angle.
          const float nextAngle = FMath::Min(angle + segmentDegrees, endAngle);
          const FVector2D outerA = circlePosition(angle, outerRadius);
          const FVector2D outerB = circlePosition(nextAngle, outerRadius);
          const FVector2D outerAST = texturePosition(angle, outerRadius);
          const FVector2D outerBST = texturePosition(nextAngle, outerRadius);

          // If the inner radius is nearly zero
          if (FMath::IsNearlyZero(innerRadius))
          {
            // Generate a single triangle for the outer edge of the textured circle segment.
            addVertex(center, centerST);
            addVertex(outerA, outerAST);
            addVertex(outerB, outerBST);
          }
          else
          {
            // Generate the triangle vertices for the inner and outer edges of the textured circle segment.
            const FVector2D innerA = circlePosition(angle, innerRadius);
            const FVector2D innerB = circlePosition(nextAngle, innerRadius);
            const FVector2D innerAST = texturePosition(angle, innerRadius);
            const FVector2D innerBST = texturePosition(nextAngle, innerRadius);
            addVertex(outerA, outerAST);
            addVertex(innerA, innerAST);
            addVertex(innerB, innerBST);
            addVertex(innerB, innerBST);
            addVertex(outerB, outerBST);
            addVertex(outerA, outerAST);
          }
        }
      }

      cache.nCachedGeometryRevision = cache.nGeometryRevision;
      return &cache;
    }

    void CUnrealCigiSymbolManager::InvalidateCircleGeometry(sbio::symbol::SymbolID symbolID)
    {
      ++CircleGeometryCaches.FindOrAdd(symbolID.Value()).nGeometryRevision;
    }

    const FSymbolGeometryCache* CUnrealCigiSymbolManager::GetCircleGeometry(sbio::symbol::SymbolID symbolID)
    {
      // Retrieve the cached circle geometry for the given symbol ID. If it does not exist, create a new cache entry.
      FSymbolGeometryCache& cache = CircleGeometryCaches.FindOrAdd(symbolID.Value());

      // If the cached geometry revision is equal to the current geometry revision, return the cached geometry.
      if (cache.nCachedGeometryRevision == cache.nGeometryRevision)
      {
        return &cache;
      }

      // Retrieve the circle geometry for the given symbol ID. If it does not exist, return nullptr.
      const sbio::symbol::CSymbolCircle* circle = FindGeometry<sbio::symbol::CSymbolCircle>(symbolID, sbio::symbol::ESymbolType::CIRCLE);
      if (circle == nullptr)
      {
        return nullptr;
      }

      SlateCustomVerts localVerts(FVector2D::ZeroVector, FVector2D(1, 1));
      localVerts.RemoveUVGrid();
      const sbio::symbol::SSymbolCircle& properties = circle->GetProperties();

      // Iterate through each circle element in the properties and generate the triangle vertices for rendering.
      for (const sbio::symbol::SCircleProperties& element : properties.circles)
      {
        const FVector2D center(element.centerUV.U, element.centerUV.V);
        if (properties.eDrawingStyle == sbio::symbol::EDrawingStyle::FILL)
        {
          localVerts.AddCircle(FColor::White, center, element.fRadius, element.fInnerRadius, element.startAngle.Value(), element.endAngle.Value());
        }
        else
        {
          localVerts.AddCircle(FColor::White, center, element.fRadius + properties.fLineWidth / 2.0f, element.fRadius - properties.fLineWidth / 2.0f, element.startAngle.Value(), element.endAngle.Value(), properties.stipplePattern, properties.fStipplePatternLength);
        }
      }

      cache.TriangleVertices.Reset(localVerts.Vertices.Num());

      // Add the triangle vertices from the localVerts to the cache's TriangleVertices array, converting each FSlateVertex's position to FVector2D.
      for (const FSlateVertex& vertex : localVerts.Vertices)
      {
        cache.TriangleVertices.Add(FVector2D(vertex.Position.X, vertex.Position.Y));
      }

      cache.nCachedGeometryRevision = cache.nGeometryRevision;
      return &cache;
    }

    void CUnrealCigiSymbolManager::InvalidatePolygonGeometry(sbio::symbol::SymbolID symbolID)
    {
      ++PolygonGeometryCaches.FindOrAdd(symbolID.Value()).nGeometryRevision;
    }

    const FSymbolGeometryCache* CUnrealCigiSymbolManager::GetPolygonGeometry(sbio::symbol::SymbolID symbolID)
    {
      // Retrieve the cached polygon geometry for the given symbol ID.
      FSymbolGeometryCache& cache = PolygonGeometryCaches.FindOrAdd(symbolID.Value());

      // If the cached geometry revision is equal to the current geometry revision, return the cached geometry.
      if (cache.nCachedGeometryRevision == cache.nGeometryRevision)
      {
        return &cache;
      }

      // Retrieve the polygon geometry for the given symbol ID. If it does not exist, return nullptr.
      const sbio::symbol::CSymbolPolygon* polygon = FindGeometry<sbio::symbol::CSymbolPolygon>(symbolID, sbio::symbol::ESymbolType::POLYGON);
      if (polygon == nullptr)
      {
        return nullptr;
      }

      const sbio::symbol::SSymbolPolygon& properties = polygon->GetProperties();
      TArray<FVector2D> points;

      // Convert the vertices from sbio::math::Vec2f to FVector2D and add them to the points array.
      for (const sbio::math::Vec2f& vertex : properties.vertices)
      {
        points.Add(FVector2D(vertex.x(), vertex.y()));
      }

      SlateCustomVerts localVerts(FVector2D::ZeroVector, FVector2D(1, 1));
      localVerts.RemoveUVGrid();

      // Generate the triangle vertices for the polygon based on the primitive type specified in the properties.
      if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::LINE)
      {
        localVerts.AddLine(FColor::White, points, false, false, properties.fLineWidth, properties.nStipplePattern, properties.fStipplePatternLength);
      }
      else if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::LINE_LOOP)
      {
        localVerts.AddLine(FColor::White, points, true, true, properties.fLineWidth, properties.nStipplePattern, properties.fStipplePatternLength);
      }
      else if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::LINE_STRIP)
      {
        localVerts.AddLine(FColor::White, points, true, false, properties.fLineWidth, properties.nStipplePattern, properties.fStipplePatternLength);
      }
      else if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::POINT)
      {
        for (const FVector2D& point : points)
        {
          localVerts.AddCircle(FColor::White, point, properties.fLineWidth / 2.0f);
        }
      }
      else if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::TRIANGLE)
      {
        for (int32 i = 2; i < points.Num(); i += 3)
        {
          localVerts.AddTriangle(FColor::White, points[i - 2], points[i - 1], points[i]);
        }
      }
      else if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::TRIANGLE_STRIP)
      {
        for (int32 i = 2; i < points.Num(); ++i)
        {
          localVerts.AddTriangle(FColor::White, points[i - 2], points[i - 1], points[i]);
        }
      }
      else if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::TRIANGLE_FAN)
      {
        for (int32 i = 2; i < points.Num(); ++i)
        {
          localVerts.AddTriangle(FColor::White, points[0], points[i - 1], points[i]);
        }
      }

      cache.TriangleVertices.Reset(localVerts.Vertices.Num());

      // Add the triangle vertices from the localVerts to the cache's TriangleVertices array, converting each FSlateVertex's position to FVector2D.
      for (const FSlateVertex& vertex : localVerts.Vertices)
      {
        cache.TriangleVertices.Add(FVector2D(vertex.Position.X, vertex.Position.Y));
      }

      cache.nCachedGeometryRevision = cache.nGeometryRevision;
      return &cache;
    }

    void CUnrealCigiSymbolManager::InvalidateTexturedPolygonGeometry(sbio::symbol::SymbolID symbolID)
    {
      ++TexturedPolygonGeometryCaches.FindOrAdd(symbolID.Value()).nGeometryRevision;
    }

    const FTexturedPolygonGeometryCache* CUnrealCigiSymbolManager::GetTexturedPolygonGeometry(sbio::symbol::SymbolID symbolID)
    {
      // Retrieve the cached textured polygon geometry for the given symbol ID.
      FTexturedPolygonGeometryCache& cache = TexturedPolygonGeometryCaches.FindOrAdd(symbolID.Value());

      // If the cached geometry revision is equal to the current geometry revision, return the cached geometry.
      if (cache.nCachedGeometryRevision == cache.nGeometryRevision)
      {
        return &cache;
      }

      // Retrieve the textured polygon geometry for the given symbol ID. If it does not exist, return nullptr.
      const sbio::symbol::CSymbolTexturedPolygon* polygon = FindGeometry<sbio::symbol::CSymbolTexturedPolygon>(symbolID, sbio::symbol::ESymbolType::TEXTURED_POLYGON);
      if (polygon == nullptr)
      {
        return nullptr;
      }

      // Retrieve the properties of the textured polygon and update the cache with the texture ID, filter mode, and wrap mode.
      const sbio::symbol::SSymbolTexturedPolygon& properties = polygon->GetProperties();
      cache.TriangleVertices.Reset();
      cache.TextureID = properties.textureID;
      cache.FilterMode = properties.eTextureFilterMode;
      cache.WrapMode = properties.eTextureWrapMode;

      // Define a lambda function to add a vertex to the triangle vertices cache using the specified index from the properties' vertices.
      const auto addVertex = [&cache, &properties](size_t index)
      {
        FTexturedSymbolVertex vertex;
        vertex.Position = FVector2D(properties.vertices[index].uv.x(), properties.vertices[index].uv.y());
        vertex.TextureST = FVector2D(properties.vertices[index].textureCoordinateST.x(), properties.vertices[index].textureCoordinateST.y());
        cache.TriangleVertices.Add(vertex);
      };

      // Define a lambda function to add a triangle to the triangle vertices cache by adding three vertices in the specified order.
      const auto addTriangle = [&addVertex](size_t a, size_t b, size_t c)
      {
        addVertex(a);
        addVertex(b);
        addVertex(c);
      };

      // Generate the triangle vertices for the textured polygon based on the primitive type specified in the properties.
      if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::TRIANGLE)
      {
        for (size_t i = 2; i < properties.vertices.size(); i += 3)
        {
          addTriangle(i - 2, i - 1, i);
        }
      }
      else if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::TRIANGLE_STRIP)
      {
        for (size_t i = 2; i < properties.vertices.size(); ++i)
        {
          addTriangle(i - 2, i - 1, i);
        }
      }
      else if (properties.ePrimitiveType == sbio::symbol::EPrimitiveType::TRIANGLE_FAN)
      {
        for (size_t i = 2; i < properties.vertices.size(); ++i)
        {
          addTriangle(0, i - 1, i);
        }
      }

      cache.nCachedGeometryRevision = cache.nGeometryRevision;
      return &cache;
    }

    void CUnrealCigiSymbolManager::InvalidateText(sbio::symbol::SymbolID symbolID)
    {
      ++TextCaches.FindOrAdd(symbolID.Value()).TextRevision;
    }

    FSymbolTextCache* CUnrealCigiSymbolManager::GetText(sbio::symbol::SymbolID symbolID)
    {
      // Retrieve the cached text for the given symbol ID.
      FSymbolTextCache& cache = TextCaches.FindOrAdd(symbolID.Value());
      const auto measureText = [this, &cache]()
      {
        if (cache.bMeasurementValid || !FSlateApplication::IsInitialized())
        {
          return;
        }

        UFont* font = FindFont(cache.FontID);
        if (font == nullptr && GEngine != nullptr)
        {
          font = GEngine->GetSmallFont();
        }

        if (font != nullptr)
        {
          float fontSize = 10.0f;

          if (cache.fFontSize > 0)
          {
            fontSize = cache.fFontSize;
          }

          const FSlateFontInfo fontInfo(font, fontSize);
          cache.MeasuredSize = FSlateApplication::Get().GetRenderer()->GetFontMeasureService()->Measure(FText::FromString(cache.DisplayString), fontInfo);
          cache.bMeasurementValid = true;
        }
      };

      // If the cached text revision is equal to the current text revision, return the cached text.
      if (cache.CachedTextRevision == cache.TextRevision)
      {
        measureText();
        return &cache;
      }

      // Retrieve the text geometry for the given symbol ID. If it does not exist, return nullptr.
      const sbio::symbol::CSymbolText* text = FindGeometry<sbio::symbol::CSymbolText>(symbolID, sbio::symbol::ESymbolType::TEXT);
      if (text == nullptr)
      {
        return nullptr;
      }

      const sbio::symbol::SSymbolTextDefinition& properties = text->GetProperties();
      const FString source = UTF8_TO_TCHAR(properties.sText.c_str());
      cache.DisplayString.Reset();

      // If the text orientation is top-to-bottom or bottom-to-top, split the source string into lines and append them to the display string with newline characters. Otherwise, set the display string to the source string directly.
      if (properties.eTextOrientation == sbio::symbol::ETextOrientation::TOP_TO_BOTTOM || properties.eTextOrientation == sbio::symbol::ETextOrientation::BOTTOM_TO_TOP)
      {
        for (int32 i = 0; i < source.Len(); ++i)
        {
          cache.DisplayString.AppendChar(source[i]);
          if (i + 1 < source.Len())
          {
            cache.DisplayString.AppendChar('\n');
          }
        }
      }
      else
      {
        cache.DisplayString = source;
      }

      cache.FontID = properties.fontID;
      cache.fFontSize = properties.fFontSize;
      cache.Alignment = properties.eTextAlignment;
      cache.Orientation = properties.eTextOrientation;
      cache.bMeasurementValid = false;
      cache.CachedTextRevision = cache.TextRevision;
      measureText();
      return &cache;
    }

    bool CUnrealCigiSymbolManager::CreateSymbolFromTemplate(sbio::symbol::SymbolID symbolID, const USymbolConfig& symbolTemplate)
    {
      // Validate the serialized symbol value before creating runtime geometry.
      const sbio::symbol::ESymbolType runtimeSymbolType = symbolTemplate.GetSymbolType();
      if (runtimeSymbolType == sbio::symbol::ESymbolType::UNKNOWN)
      {
        return false;
      }

      if (!symbolTemplate.ValidateSymbol().IsEmpty())
      {
        return false;
      }

      // SimulationSDK creates a TEMPLATE placeholder before dispatching the creation callback.
      // Replace that placeholder, but do not overwrite an existing concrete symbol.
      sbio::symbol::CSymbol* existingSymbol = FindSymbol(symbolID);
      if (existingSymbol != nullptr)
      {
        if (existingSymbol->GetSymbolType() != sbio::symbol::ESymbolType::TEMPLATE || SymbolSurfaceManager == nullptr)
        {
          return false;
        }
        RemoveSymbol(symbolID);
      }

      // Check if the symbol template is for a textured circle and if the type is within the valid range of primitive types for textured circles.
      if (runtimeSymbolType == sbio::symbol::ESymbolType::CIRCLE && (symbolTemplate.GetDrawingStyle() == sbio::symbol::EDrawingStyle::LINE || symbolTemplate.GetDrawingStyle() == sbio::symbol::EDrawingStyle::FILL))
      {
        // Create a new circle symbol with the specified symbol ID and type (CIRCLE) using the CreateGeometry function.
        sbio::symbol::CSymbolCircle* circle = CreateGeometry<sbio::symbol::CSymbolCircle>(symbolID, sbio::symbol::ESymbolType::CIRCLE);

        // Check if the circle was successfully created. If not, return false to indicate that the circle cannot be created.
        if (circle == nullptr)
        {
          RemoveSymbol(symbolID);
          return false;
        }

        sbio::symbol::SSymbolCircle properties = circle->GetProperties();
        properties.eDrawingStyle = sbio::symbol::EDrawingStyle::LINE;

        // If the symbol template type is 1, set the drawing style to FILL to indicate that the circle should be filled.
        if (symbolTemplate.GetDrawingStyle() == sbio::symbol::EDrawingStyle::FILL)
        {
          properties.eDrawingStyle = sbio::symbol::EDrawingStyle::FILL;
        }

        properties.fLineWidth = symbolTemplate.lineWidth;
        properties.stipplePattern = symbolTemplate.stipple;
        properties.fStipplePatternLength = symbolTemplate.stippleLength;
        properties.circles.clear();
        const int32 count = FMath::Min3(symbolTemplate.vertices.Num(), symbolTemplate.radii.Num(), symbolTemplate.angles.Num());

        // Iterate through the vertices, radii, and angles of the symbol template to create circle properties for each element and add them to the properties' circles array.
        for (int32 i = 0; i < count; ++i)
        {
          sbio::symbol::SCircleProperties element;
          element.centerUV.U = symbolTemplate.vertices[i].X;
          element.centerUV.V = symbolTemplate.vertices[i].Y;
          element.fRadius = symbolTemplate.radii[i].X;
          element.fInnerRadius = 0;

          if (symbolTemplate.GetDrawingStyle() == sbio::symbol::EDrawingStyle::FILL)
          {
            element.fInnerRadius = symbolTemplate.radii[i].Y;
          }

          element.startAngle = sbio::math::Degrees(symbolTemplate.angles[i].X);
          element.endAngle = sbio::math::Degrees(symbolTemplate.angles[i].Y);
          properties.circles.push_back(element);
        }

        // Set the properties of the circle and invalidate the cached geometry for the symbol ID.
        circle->Set(properties, symbolID);
        InvalidateCircleGeometry(symbolID);
        return true;
      }

      // Check if the symbol template is for a polygon and if the type is within the valid range of primitive types for polygons.
      if (runtimeSymbolType == sbio::symbol::ESymbolType::POLYGON && static_cast<int32>(symbolTemplate.GetPrimitiveType()) >= static_cast<int32>(sbio::symbol::EPrimitiveType::POINT) && static_cast<int32>(symbolTemplate.GetPrimitiveType()) <= static_cast<int32>(sbio::symbol::EPrimitiveType::TRIANGLE_FAN))
      {
        sbio::symbol::CSymbolPolygon* polygon = CreateGeometry<sbio::symbol::CSymbolPolygon>(symbolID, sbio::symbol::ESymbolType::POLYGON);

        // Check if the polygon was successfully created. If not, return false to indicate that the polygon cannot be created.
        if (polygon == nullptr)
        {
          RemoveSymbol(symbolID);
          return false;
        }

        sbio::symbol::SSymbolPolygon properties = polygon->GetProperties();
        properties.ePrimitiveType = static_cast<sbio::symbol::EPrimitiveType>(symbolTemplate.GetPrimitiveType());
        properties.fLineWidth = symbolTemplate.lineWidth;
        properties.nStipplePattern = symbolTemplate.stipple;
        properties.fStipplePatternLength = symbolTemplate.stippleLength;
        properties.vertices.clear();

        // Convert the vertices from FVector2D to sbio::math::Vec2f and add them to the properties' vertices array.
        for (const FVector2D& vertex : symbolTemplate.vertices)
        {
          properties.vertices.push_back(sbio::math::Vec2f(vertex.X, vertex.Y));
        }

        // Set the properties of the polygon and invalidate the cached geometry for the symbol ID.
        polygon->Set(properties, symbolID);
        InvalidatePolygonGeometry(symbolID);
        return true;
      }

      // Check if the symbol template is for a textured circle and if the type is within the valid range of primitive types for textured circles.
      if (runtimeSymbolType == sbio::symbol::ESymbolType::TEXTURED_CIRCLE)
      {
        sbio::symbol::CSymbolTexturedCircle* circle = CreateGeometry<sbio::symbol::CSymbolTexturedCircle>(symbolID, sbio::symbol::ESymbolType::TEXTURED_CIRCLE);

        // Check if the circle was successfully created. If not, return false to indicate that the textured circle cannot be created.
        if (circle == nullptr)
        {
          RemoveSymbol(symbolID);
          return false;
        }

        sbio::symbol::SSymbolTexturedCircle properties = circle->GetProperties();
        properties.textureID = sbio::TextureID(symbolTemplate.textureID);
        properties.eTextureFilter = static_cast<sbio::symbol::ETextureFilterMode>(symbolTemplate.textureFilterMode);
        properties.eTextureWrap = static_cast<sbio::symbol::ETextureWrapMode>(symbolTemplate.textureWrapMode);
        properties.circles.clear();

        for (const FTexturedCircleTemplateElement& templateElement : symbolTemplate.texturedCircles)
        {
          sbio::symbol::STexturedCircleProperties element;
          element.centerUV = sbio::math::Vec2f(templateElement.centerUV.X, templateElement.centerUV.Y);
          element.fRadius = templateElement.radius;
          element.fInnerRadius = templateElement.innerRadius;
          element.startAngle = sbio::math::Degrees(templateElement.angles.X);
          element.endAngle = sbio::math::Degrees(templateElement.angles.Y);
          element.centerTextureST = sbio::math::Vec2f(templateElement.centerTextureST.X, templateElement.centerTextureST.Y);
          element.fTextureMapRadius = templateElement.textureMapRadius;
          element.fTextureMapRotation = templateElement.textureMapRotation;
          properties.circles.push_back(element);
        }

        // Set the properties of the textured circle and invalidate the cached geometry for the symbol ID.
        circle->Set(properties, symbolID);
        InvalidateTexturedCircleGeometry(symbolID);
        return true;
      }

      // Check if the symbol template is for a textured polygon and if the type is within the valid range of primitive types for textured polygons.
      if (runtimeSymbolType == sbio::symbol::ESymbolType::TEXTURED_POLYGON && static_cast<int32>(symbolTemplate.GetPrimitiveType()) >= static_cast<int32>(sbio::symbol::EPrimitiveType::POINT) && static_cast<int32>(symbolTemplate.GetPrimitiveType()) <= static_cast<int32>(sbio::symbol::EPrimitiveType::TRIANGLE_FAN))
      {
        // Create a new textured polygon symbol with the specified symbol ID and type (TEXTURED_POLYGON) using the CreateGeometry function.
        sbio::symbol::CSymbolTexturedPolygon* polygon = CreateGeometry<sbio::symbol::CSymbolTexturedPolygon>(symbolID, sbio::symbol::ESymbolType::TEXTURED_POLYGON);

        // Check if the polygon was successfully created. If not, return false to indicate that the textured polygon cannot be created.
        if (polygon == nullptr)
        {
          RemoveSymbol(symbolID);
          return false;
        }

        sbio::symbol::SSymbolTexturedPolygon properties = polygon->GetProperties();
        properties.textureID = sbio::TextureID(symbolTemplate.textureID);
        properties.ePrimitiveType = static_cast<sbio::symbol::EPrimitiveType>(symbolTemplate.GetPrimitiveType());
        properties.eTextureFilterMode = static_cast<sbio::symbol::ETextureFilterMode>(symbolTemplate.textureFilterMode);
        properties.eTextureWrapMode = static_cast<sbio::symbol::ETextureWrapMode>(symbolTemplate.textureWrapMode);
        properties.vertices.clear();

        // Convert the textured polygon vertices from FTexturedPolygonTemplateVertex to sbio::symbol::SSymbolTexturedPolygonVertex
        // and add them to the properties' vertices array.
        for (const FTexturedPolygonTemplateVertex& templateVertex : symbolTemplate.texturedPolygonVertices)
        {
          sbio::symbol::SSymbolTexturedPolygonVertex vertex;
          vertex.uv = sbio::math::Vec2f(templateVertex.uv.X, templateVertex.uv.Y);
          vertex.textureCoordinateST = sbio::math::Vec2f(templateVertex.textureCoordinateST.X, templateVertex.textureCoordinateST.Y);
          properties.vertices.push_back(vertex);
        }

        // Set the properties of the textured polygon and invalidate the cached geometry for the symbol ID.
        polygon->Set(properties, symbolID);
        InvalidateTexturedPolygonGeometry(symbolID);
        return true;
      }

      return false;
    }

    void CUnrealCigiSymbolManager::SetProjectFonts(const TArray<UFont*>& projectFonts)
    {
      // Reset the ProjectFonts array to match the number of provided project fonts.
      ProjectFonts.Reset(projectFonts.Num());

      // Iterate through the provided project fonts.
      for (UFont* font : projectFonts)
      {
        // Check if the font is valid before adding it to the ProjectFonts array. If it is valid, add it to the array.  
        if (IsValid(font))
        {
          ProjectFonts.Add(font);
        }
      }
    }

    bool CUnrealCigiSymbolManager::RegisterFontByName(sbio::FontID fontID, const FString& fontName)
    {
      // Check if the font ID is already registered in the Fonts map. If it is, return false to indicate that the font cannot be registered again.
      if (fontID == sbio::UnknownFontID || Fonts.Contains(fontID.Value()))
      {
        return false;
      }

      // Iterate through the ProjectFonts array..
      for (const TWeakObjectPtr<UFont>& fontReference : ProjectFonts)
      {
        UFont* font = fontReference.Get();

        // Check if the font is valid and if its name matches the specified font name. If a match is found, add the font to the Fonts map and return true.
        if (IsValid(font) && font->GetName().Equals(fontName))
        {
          Fonts.Add(fontID.Value(), font);
          return true;
        }
      }

      return false;
    }

    UFont* CUnrealCigiSymbolManager::FindFont(sbio::FontID fontID) const
    {
      // Find the font associated with the given font ID in the Fonts map.
      const TWeakObjectPtr<UFont>* font = Fonts.Find(fontID.Value());

      // If the font is not found in the Fonts map
      if (font == nullptr)
      {
        return nullptr;
      }
      
      return font->Get();
    }

    void CUnrealCigiSymbolManager::ClearFonts()
    {
      Fonts.Empty();
      ProjectFonts.Empty();
    }

    void CUnrealCigiSymbolManager::RemoveRenderCaches(sbio::symbol::SymbolID symbolID)
    {
      TexturedCircleGeometryCaches.Remove(symbolID.Value());
      CircleGeometryCaches.Remove(symbolID.Value());
      PolygonGeometryCaches.Remove(symbolID.Value());
      TexturedPolygonGeometryCaches.Remove(symbolID.Value());
      TextCaches.Remove(symbolID.Value());
    }

    void CUnrealCigiSymbolManager::ClearRenderCaches()
    {
      TexturedCircleGeometryCaches.Empty();
      CircleGeometryCaches.Empty();
      PolygonGeometryCaches.Empty();
      TexturedPolygonGeometryCaches.Empty();
      TextCaches.Empty();
    }
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026