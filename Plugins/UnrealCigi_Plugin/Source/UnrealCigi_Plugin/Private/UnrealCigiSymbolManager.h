//Copyright SimBlocks LLC 2016-2026
#pragma once

#include "CigiSymbol.h"
#include "IGCigiLib/CigiSymbol.h"
#include "SymbolLib/SymbolSurfaceManager.h"
#include "Engine/Texture2D.h"
#include "UObject/StrongObjectPtr.h"
#include <memory>

class UFont;
class USymbolConfig;

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @class CUnrealCigiSymbolManager
     * @brief Maintains symbol surfaces and their Unreal rendering resources.
     */
    class CUnrealCigiSymbolManager
    {
    public:
      /**
       * @brief Constructor for CUnrealCigiSymbolManager.
       * @param symbolSurfaceManager Shared pointer to the symbol surface manager.
       */
      explicit CUnrealCigiSymbolManager(const std::shared_ptr<sbio::symbol::CSymbolSurfaceManager>& symbolSurfaceManager);

      /** @brief Releases manager-owned rooted symbol templates. */
      ~CUnrealCigiSymbolManager();

      /**
       * @brief Finds a symbol by its ID.
       * @param symbolID The ID of the symbol to find.
       * @return Pointer to the found symbol, or nullptr if not found.
       */
      sbio::symbol::CSymbol* FindSymbol(sbio::symbol::SymbolID symbolID) const;

      /**
       * @brief Finds a symbol by its ID and type.
       * @param symbolID The ID of the symbol to find.
       * @param symbolType The type of the symbol to find.
       * @return Pointer to the found symbol, or nullptr if not found.
       */
      sbio::symbol::CSymbol* FindSymbol(sbio::symbol::SymbolID symbolID, sbio::symbol::ESymbolType symbolType) const;

      /**
       * @brief Creates a new symbol.
       * @param symbolID The ID of the symbol to create.
       * @param symbolType The type of the symbol to create.
       * @return Pointer to the created symbol.
       */
      sbio::symbol::CSymbol* CreateSymbol(sbio::symbol::SymbolID symbolID, sbio::symbol::ESymbolType symbolType);

      /**
       * @brief Removes a symbol and its Unreal render caches.
       * @param symbolID Identifier of the symbol to remove.
       */
      void RemoveSymbol(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Finds geometry of a specific type for a given symbol ID and type.
       * @param symbolID The ID of the symbol.
       * @param symbolType The type of the symbol.
       * @return Pointer to the geometry of the symbol, or nullptr if not found or cast is invalid.
       */
      template <typename TGeometry>
      TGeometry* FindGeometry(sbio::symbol::SymbolID symbolID, sbio::symbol::ESymbolType symbolType) const
      {
        // Attempt to find the symbol with the given ID and type
        sbio::symbol::CSymbol* symbol = FindSymbol(symbolID, symbolType);

        // If the symbol is not found, return nullptr
        if (symbol == nullptr)
        {
          return nullptr;
        }

        // Attempt to cast the symbol's geometry to the requested type
        return dynamic_cast<TGeometry*>(symbol->GetSymbolGeometry());
      }

      /**
       * @brief Creates geometry of a specific type for a given symbol ID and type.
       * @param symbolID The ID of the symbol.
       * @param symbolType The type of the symbol.
       * @return Pointer to the created geometry of the symbol, or nullptr if not found or cast is invalid.
       */
      template <typename TGeometry>
      TGeometry* CreateGeometry(sbio::symbol::SymbolID symbolID, sbio::symbol::ESymbolType symbolType)
      {
        sbio::symbol::CSymbol* symbol = CreateSymbol(symbolID, symbolType);
        if (symbol == nullptr)
        {
          return nullptr;
        }
        return dynamic_cast<TGeometry*>(symbol->GetSymbolGeometry());
      }

      /**
       * @brief Finds a surface by its ID.
       * @param surfaceID The ID of the surface to find.
       * @return Pointer to the found surface, or nullptr if not found.
       */
      FUnrealSymbolSurface* FindSurface(sbio::symbol::SymbolSurfaceID surfaceID);

      /**
       * @brief Creates a surface.
       * @param surfaceID The ID of the surface to create.
       * @return Reference to the created surface.
       */
      FUnrealSymbolSurface& CreateSurface(sbio::symbol::SymbolSurfaceID surfaceID);

      /**
       * @brief Updates a billboard surface with new data.
       * @param data The data to update the billboard surface with.
       * @return Pointer to the updated surface, or nullptr if the update fails.
       */
      FUnrealSymbolSurface* UpdateBillboardSurface(const sbio::ig::symbol::SUpdateEntityBillboardSymbolSurfaceMessage& data);

      /**
       * @brief Updates a world surface with new data.
       * @param data The data to update the world surface with.
       * @return Pointer to the updated surface, or nullptr if the update fails.
       */
      FUnrealSymbolSurface* UpdateWorldSurface(const sbio::ig::symbol::SUpdateSymbolSurfaceMessage& data);

      /**
       * @brief Updates a view surface with new data.
       * @param data The data to update the view surface with.
       * @return Pointer to the updated surface, or nullptr if the update fails.
       */
      FUnrealSymbolSurface* UpdateViewSurface(const sbio::ig::symbol::SUpdateViewSymbolSurfaceMessage& data);

      /**
       * @brief Removes a surface by its ID.
       * @param surfaceID The ID of the surface to remove.
       */
      void RemoveSurface(sbio::symbol::SymbolSurfaceID surfaceID);

      /**
       * @brief Finds all symbols on a surface.
       * @param surfaceID The ID of the surface.
       * @return Array of symbol IDs located on the surface.
       */
      TArray<sbio::symbol::SymbolID> FindSymbolsOnSurface(sbio::symbol::SymbolSurfaceID surfaceID) const;

      /**
       * @brief Registers a texture with a specific ID.
       * @param textureID The ID to register the texture with.
       * @param texture Pointer to the texture to register.
       * @return True if the texture was registered, false if the ID or texture is invalid.
       */
      bool RegisterTexture(sbio::TextureID textureID, UTexture2D* texture);

      /**
       * @brief Finds a registered texture by its ID.
       * @param textureID The ID of the texture to find.
       * @return Pointer to the found texture, or nullptr if not found.
       */
      UTexture2D* FindTexture(sbio::TextureID textureID) const;

      /**
       * @brief Clears all registered textures.
       */
      void ClearTextures();

      /**
       * @brief Clears all surfaces and associated resources.
       */
      void ClearSurfaces();

      /**
       * @brief Invalidates the textured circle geometry for a specific symbol ID.
       * @param symbolID The ID of the symbol whose geometry is to be invalidated.
       */
      void InvalidateTexturedCircleGeometry(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Gets the textured circle geometry for a specific symbol ID.
       * @param symbolID The ID of the symbol.
       * @return Pointer to the cached textured circle geometry, or nullptr if not found.
       */
      const FTexturedCircleGeometryCache* GetTexturedCircleGeometry(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Invalidates the circle geometry for a specific symbol ID.
       * @param symbolID The ID of the symbol whose geometry is to be invalidated.
       */
      void InvalidateCircleGeometry(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Gets the circle geometry for a specific symbol ID.
       * @param symbolID The ID of the symbol.
       * @return Pointer to the cached circle geometry, or nullptr if not found.
       */
      const FSymbolGeometryCache* GetCircleGeometry(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Invalidates the polygon geometry for a specific symbol ID.
       * @param symbolID The ID of the symbol whose geometry is to be invalidated.
       */
      void InvalidatePolygonGeometry(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Gets the polygon geometry for a specific symbol ID.
       * @param symbolID The ID of the symbol.
       * @return Pointer to the cached polygon geometry, or nullptr if not found.
       */
      const FSymbolGeometryCache* GetPolygonGeometry(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Invalidates the textured polygon geometry for a specific symbol ID.
       * @param symbolID The ID of the symbol whose geometry is to be invalidated.
       */
      void InvalidateTexturedPolygonGeometry(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Gets the textured polygon geometry for a specific symbol ID.
       * @param symbolID The ID of the symbol.
       * @return Pointer to the cached textured polygon geometry, or nullptr if not found.
       */
      const FTexturedPolygonGeometryCache* GetTexturedPolygonGeometry(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Invalidates the text cache for a specific symbol ID.
       * @param symbolID The ID of the symbol whose text cache is to be invalidated.
       */
      void InvalidateText(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Gets the text cache for a specific symbol ID.
       * @param symbolID The ID of the symbol.
       * @return Pointer to the cached text, or nullptr if not found.
       */
      FSymbolTextCache* GetText(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Removes all render caches associated with a symbol ID.
       * @param symbolID The ID of the symbol whose render caches are to be removed.
       */
      void RemoveRenderCaches(sbio::symbol::SymbolID symbolID);

      /**
       * @brief Clears all render caches.
       */
      void ClearRenderCaches();

      /**
       * @brief Creates a symbol from a template.
       * @param symbolID The ID of the symbol to create.
       * @param symbolTemplate The template to create the symbol from.
       * @return True if the symbol was successfully created from the template, false otherwise.
       */
      bool CreateSymbolFromTemplate(sbio::symbol::SymbolID symbolID, const USymbolConfig& symbolTemplate);

      /**
       * @brief Registers a symbol template.
       *
       * The manager accepts only unrooted templates and assumes root ownership of
       * successfully registered templates until they are cleared.
       * @param templateID The ID to register the template with.
       * @param symbolTemplate Pointer to the symbol config template to register.
       * @return True if the template was registered, or false if the ID or template is invalid,
       * already registered, or already rooted.
       */
      bool RegisterSymbolTemplate(int32 templateID, USymbolConfig* symbolTemplate);

      /**
       * @brief Finds a symbol template by its ID.
       * @param templateID The ID of the template to find.
       * @return Pointer to the found symbol config template, or nullptr if not found.
       */
      USymbolConfig* FindSymbolTemplate(int32 templateID) const;

      /**
       * @brief Clears all registered symbol templates.
       */
      void ClearSymbolTemplates();

      /**
       * @brief Sets the project-wide fonts.
       * @param projectFonts Array of pointers to the fonts to set as project fonts.
       */
      void SetProjectFonts(const TArray<UFont*>& projectFonts);

      /**
       * @brief Registers a font by its name.
       * @param fontID The ID to register the font with.
       * @param fontName The name of the font to register.
       * @return True if the font was successfully registered, false otherwise.
       */
      bool RegisterFontByName(sbio::FontID fontID, const FString& fontName);

      /**
       * @brief Finds a registered font by its ID.
       * @param fontID The ID of the font to find.
       * @return Pointer to the found font, or nullptr if not found.
       */
      UFont* FindFont(sbio::FontID fontID) const;

      /**
       * @brief Clears all registered fonts.
       */
      void ClearFonts();

    private:
      std::shared_ptr<sbio::symbol::CSymbolSurfaceManager> SymbolSurfaceManager; //!< Shared pointer to the symbol surface manager.
      TMap<int32, FUnrealSymbolSurface> UnrealSurfaces; //!< Map of registered Unreal surfaces.
      TMap<int32, TStrongObjectPtr<UTexture2D>> Textures; //!< Map of registered textures.
      TMap<int32, USymbolConfig*> SymbolTemplates; //!< Map of registered symbol templates.
      TMap<int32, FTexturedCircleGeometryCache> TexturedCircleGeometryCaches; //!< Cache for textured circle geometries.
      TMap<int32, FSymbolGeometryCache> CircleGeometryCaches; //!< Cache for circle geometries.
      TMap<int32, FSymbolGeometryCache> PolygonGeometryCaches; //!< Cache for polygon geometries.
      TMap<int32, FTexturedPolygonGeometryCache> TexturedPolygonGeometryCaches; //!< Cache for textured polygon geometries.
      TMap<int32, FSymbolTextCache> TextCaches; //!< Cache for symbol texts.
      TArray<TWeakObjectPtr<UFont>> ProjectFonts; //!< Array of project-wide fonts.
      TMap<int32, TWeakObjectPtr<UFont>> Fonts; //!< Map of registered fonts.
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026