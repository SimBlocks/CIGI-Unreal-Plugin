//Copyright SimBlocks LLC 2016-2026

#pragma once

#include "IGCigiLib/IGCigiLib.h"
#include "GlobalHeaders/Globals.h"
#include "EntityLib/EntityDeclarations.h"
#include "ViewLib/ViewDeclarations.h"
#include "SymbolLib/SymbolDeclarations.h"
#include "IGCigiLib/IGCigiTypeDeclarations.h"
#include "ViewLib/ViewTypes.h"
#include "IGCigiLib/ImageGenerator.h"
#include "UnrealCigi_Declarations.h"

#ifdef GetObject
#undef GetObject
#endif

#include "UnrealCigiEventHandler.h"
#include "UnrealCigiDatabaseManager.h"
#include "UnrealCigiEnvironmentManager.h"
#include "UnrealCigiEntityManager.h"
#include "UnrealCigiViewManager.h"
#include "UnrealCigiPhysicsManager.h"
#include "UnrealCigiComponentDispatcher.h"
#include "UnrealCigiSymbolManager.h"
#include "UnrealCigiSymbolSurfacePresenter.h"

namespace sbio
{
  namespace unrealcigi
  {
    /**
     * @struct SUnrealCigiGlobals
     * @brief Stores global UnrealCigi plugin state shared by internal components.
     */
    struct SUnrealCigiGlobals : sbio::SGlobals
    {
      /** Entity manager shared by the plugin. */
      std::shared_ptr<sbio::entity::CEntityManager> pEntityManager;
      /** View manager shared by the plugin. */
      std::shared_ptr<sbio::view::CViewManager> pViewManager;
      /** Symbol-surface manager shared by the plugin. */
      std::shared_ptr<sbio::symbol::CSymbolSurfaceManager> pSymbolSurfaceManager;
      /** UnrealCigi event handler. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiEventHandler> pEventHandler;
      /** UnrealCigi database manager. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiDatabaseManager> pDatabaseManager;
      /** UnrealCigi environment manager. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiEnvironmentManager> pEnvironmentManager;
      /** Unreal symbol manager. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiSymbolManager> pUnrealSymbolManager;
      /** Presenter for Unreal symbol-surface widgets. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiSymbolSurfacePresenter> pSymbolSurfacePresenter;
      /** Unreal entity manager. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiEntityManager> pUnrealEntityManager;
      /** Unreal view manager. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiViewManager> pUnrealViewManager;
      /** Unreal physics manager. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiPhysicsManager> pPhysicsManager;
      /** Component-message dispatcher. */
      std::unique_ptr<sbio::unrealcigi::CUnrealCigiComponentDispatcher> pComponentDispatcher;
      /** Image-generator event messenger. */
      std::unique_ptr<sbio::ig::CImageGeneratorEventMessenger> pEventMessenger;
      /** CIGI image-generator instance. */
      std::unique_ptr<sbio::cigi::ig::CCigiImageGenerator> pImageGenerator;
      /** Dispatcher for exported-function events. */
      std::unique_ptr<sbio::cigi::ig::CIGResponseEventDispatcher> pExportedFunctionsEventDispatcher;

      ~SUnrealCigiGlobals();
      /**
       * @brief Resets the global manager and handler state.
       *
       * This function clears all managed entities, views, symbols, and other resources
       * while ensuring that the global state is properly shut down and cleaned up.
       */
      void Reset();
    };
  }
}

//The source code in this file is licensed under the MIT License. See the LICENSE text file for full terms.
//Refer all inquiries to sales@simblocks.io
//Copyright SimBlocks LLC 2016-2026